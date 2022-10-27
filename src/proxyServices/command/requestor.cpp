#include <string>
#include <filesystem>
#include "umps/proxyServices/command/requestor.hpp"
#include "umps/proxyServices/command/requestorOptions.hpp"
#include "umps/proxyServices/command/availableModulesRequest.hpp"
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/terminateRequest.hpp"
#include "umps/services/command/terminateResponse.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/failure.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/context.hpp"
#include "private/messaging/requestReplySocket.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UCommand = UMPS::Services::Command;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class Requestor::RequestorImpl : public ::RequestSocket 
{
public:
    /// @brief Constructor
    RequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger) :
         ::RequestSocket(context, logger)
    {
        // Make the message types
        std::unique_ptr<UMPS::MessageFormats::IMessage>
            availableCommandsResponse
               = std::make_unique<UCommand::AvailableCommandsResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage>
            availableModulesResponse
               = std::make_unique<AvailableModulesResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> commandsResponse
            = std::make_unique<UCommand::CommandResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> terminateResponse
            = std::make_unique<UCommand::TerminateResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> failureResponse
            = std::make_unique<UMPS::MessageFormats::Failure> ();
        mMessageFormats.add(availableCommandsResponse);
        mMessageFormats.add(availableModulesResponse);
        mMessageFormats.add(commandsResponse);
        mMessageFormats.add(terminateResponse);
        mMessageFormats.add(failureResponse);
    }
    /// @brief Sends a message to the router.  But we need to actually tell
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        requestFromModule(const std::string &moduleName,
                          const UMPS::MessageFormats::IMessage &message)
    {
        if (moduleName.empty())
        {
            throw std::invalid_argument("Module name is empty");
        }
        auto messageType = message.getMessageType(); // Throw early
        auto messageContents = message.toMessage(); // Throw early
        // We'll use two frames.  The first is, as per usual, the message type,
        // then who I want to talk to.  Basically, I don't want the proxy doing
        // unnecessary (de)serializing of messages to figure this out.
        zmq::const_buffer headerBuffer1{messageType.data(), messageType.size()};
        zmq::const_buffer headerBuffer2{moduleName.data(), moduleName.size()};
        mSocket->send(headerBuffer1, zmq::send_flags::sndmore);
        mSocket->send(headerBuffer2, zmq::send_flags::sndmore);
        // Now send the contents
        zmq::const_buffer messageBuffer{messageContents.data(),
                                        messageContents.size()};
        mSocket->send(messageBuffer, zmq::send_flags::none);
        // Finally, wait for the response
        return receive(zmq::recv_flags::none); // Now return the reply
    }
/*
    /// @brief Disconnect
    void disconnect()
    {
        mRequest->disconnect();
    }
*/
    UMPS::MessageFormats::Messages mMessageFormats;
    RequestorOptions mRequestorOptions;
    UMPS::Messaging::RequestRouter::RequestOptions mRequestOptions;
};

/// C'tor
Requestor::Requestor() :
    pImpl(std::make_unique<RequestorImpl> (nullptr, nullptr))
{
}

Requestor::Requestor(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RequestorImpl> (context, nullptr))
{
}

Requestor::Requestor(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestorImpl> (nullptr, logger))
{
}

Requestor::Requestor(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestorImpl> (context, logger))
{
}

/// Move c'tor
Requestor::Requestor(Requestor &&requestor) noexcept
{
    *this = std::move(requestor);
}

/// Move assignment
Requestor& 
Requestor::operator=(Requestor &&requestor) noexcept
{
    if (&requestor == this){return *this;}
    pImpl = std::move(requestor.pImpl);
    return *this;
}

/// Destructor
Requestor::~Requestor() = default;

/// Initialize
void Requestor::initialize(const RequestorOptions &options)
{
    pImpl->disconnect();
    auto requestOptions = options.getOptions();
    auto messageFormats = pImpl->mMessageFormats.get();
    for (auto &messageFormat : messageFormats)
    {
        requestOptions.addMessageFormat(messageFormat.second);
    }
    auto socketOptions = pImpl->convertSocketOptions(requestOptions);
    pImpl->connect(socketOptions);
    pImpl->mRequestorOptions = options;
    pImpl->mRequestOptions = requestOptions;
}

/// Initialized?
bool Requestor::isInitialized() const noexcept
{
    return pImpl->isConnected();
}

/// Available modules
std::unique_ptr<AvailableModulesResponse>
    Requestor::getAvailableModules() const
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<AvailableModulesResponse> result{nullptr};
    AvailableModulesRequest requestMessage;
    auto message = pImpl->request(requestMessage);
    if (message != nullptr)
    {
        result = static_unique_pointer_cast<AvailableModulesResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

/// Commands
std::unique_ptr<UCommand::AvailableCommandsResponse>
    Requestor::getCommands(const std::string &moduleName) const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<UCommand::AvailableCommandsResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    UCommand::AvailableCommandsRequest requestMessage;
    auto message = pImpl->requestFromModule(moduleName, requestMessage);
    if (message != nullptr)
    {
        if (message->getMessageType() == failureMessage.getMessageType())
        {
            failureMessage.fromMessage(message->toMessage());
            throw std::runtime_error("Failed to get commands.  Failed with: "
                                   + failureMessage.getDetails());
        } 
        result = static_unique_pointer_cast<UCommand::AvailableCommandsResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

std::unique_ptr<UCommand::CommandResponse>
    Requestor::issueCommand(const std::string &moduleName,
                            const UCommand::CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    std::unique_ptr<UCommand::CommandResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    auto message = pImpl->requestFromModule(moduleName, request);
    if (message != nullptr)
    {
        if (message->getMessageType() == failureMessage.getMessageType())
        {
            failureMessage.fromMessage(message->toMessage());
            throw std::runtime_error("Failed to issue command.  Failed with: "
                                   + failureMessage.getDetails());
        }
        result = static_unique_pointer_cast<UCommand::CommandResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

std::unique_ptr<UCommand::TerminateResponse>
    Requestor::issueTerminateCommand(const std::string &moduleName) const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<UCommand::TerminateResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    UCommand::TerminateRequest requestMessage;
    auto message = pImpl->requestFromModule(moduleName, requestMessage);
    if (message != nullptr)
    {
        if (message->getMessageType() == failureMessage.getMessageType())
        {
            failureMessage.fromMessage(message->toMessage());
            throw std::runtime_error(
                "Failed to issue terminate command.  Failed with: "
               + failureMessage.getDetails());
        }
        result = static_unique_pointer_cast<UCommand::TerminateResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

void Requestor::disconnect()
{
    pImpl->disconnect();
}

