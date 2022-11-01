#include <iostream>
#include <string>
#include <filesystem>
#include "umps/services/command/requestor.hpp"
#include "umps/services/command/requestorOptions.hpp"
#include "umps/proxyServices/command/requestor.hpp"
#include "umps/proxyServices/command/requestorOptions.hpp"
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
#include "umps/messageFormats/staticUniquePointerCast.hpp"
#include "private/messaging/requestReplySocket.hpp"

using namespace UMPS::Services::Command;
namespace UMF = UMPS::MessageFormats;

class Requestor::RequestorImpl : public RequestSocket
{
public:
    /// @brief Constructor
    RequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger) :
        RequestSocket(context, logger)
    {
        // Make the message types
        std::unique_ptr<UMPS::MessageFormats::IMessage>
            availableCommandsResponse
               = std::make_unique<AvailableCommandsResponse> (); 
        std::unique_ptr<UMPS::MessageFormats::IMessage> commandsResponse
            = std::make_unique<CommandResponse> (); 
        std::unique_ptr<UMPS::MessageFormats::IMessage> terminateResponse
            = std::make_unique<TerminateResponse> (); 
        std::unique_ptr<UMPS::MessageFormats::IMessage> failureResponse
            = std::make_unique<UMPS::MessageFormats::Failure> (); 
        mMessageFormats.add(availableCommandsResponse);
        mMessageFormats.add(commandsResponse);
        mMessageFormats.add(terminateResponse);
        mMessageFormats.add(failureResponse);
    }
//    std::unique_ptr<UMPS::ProxyServices::Command::Requestor>
//        mRequestor{nullptr};
    RequestorOptions mRequestorOptions;
    UMPS::MessageFormats::Messages mMessageFormats;
    std::filesystem::path mIPCFileName;
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
Requestor& Requestor::operator=(Requestor &&requestor) noexcept
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
    if (!options.haveModuleName())
    {
        throw std::invalid_argument("Module name not set");
    }
    pImpl->mIPCFileName = options.getIPCFileName();
    if (!std::filesystem::exists(pImpl->mIPCFileName))
    {
        throw std::runtime_error("IPC file " + pImpl->mIPCFileName.string()
                               + " does not exist");
    }
    UMPS::Messaging::SocketOptions socketOptions;
    auto requestOptions = options.getOptions().getOptions();
    socketOptions.setAddress(requestOptions.getAddress());
    socketOptions.setZAPOptions(requestOptions.getZAPOptions());
    socketOptions.setMessageFormats(pImpl->mMessageFormats);
    socketOptions.setSendHighWaterMark(requestOptions.getHighWaterMark());
    socketOptions.setReceiveHighWaterMark(requestOptions.getHighWaterMark());
    // Return immediately after sending
    socketOptions.setSendTimeOut(std::chrono::milliseconds {0});
    socketOptions.setReceiveTimeOut(requestOptions.getTimeOut());
    pImpl->connect(socketOptions);
    pImpl->mRequestorOptions = options;
}

/// Initialized?
bool Requestor::isInitialized() const noexcept
{
    return pImpl->isConnected();
}

/// Get available commands
std::unique_ptr<AvailableCommandsResponse> Requestor::getCommands() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<AvailableCommandsResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    AvailableCommandsRequest requestMessage;
    auto message = pImpl->request(requestMessage);
    if (message != nullptr)
    {
        if (message->getMessageType() == failureMessage.getMessageType())
        {
            failureMessage.fromMessage(message->toMessage());
            throw std::runtime_error("Failed to get commands.  Failed with: "
                                   + failureMessage.getDetails());
        }
        result = UMF::static_unique_pointer_cast<AvailableCommandsResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

/// Issue a command
std::unique_ptr<CommandResponse> Requestor::issueCommand(
    const CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    std::unique_ptr<CommandResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    auto message = pImpl->request(request);
    if (message != nullptr)
    {
        if (message->getMessageType() == failureMessage.getMessageType())
        {
            failureMessage.fromMessage(message->toMessage());
            throw std::runtime_error("Failed to issue command.  Failed with: "
                                   + failureMessage.getDetails());
        }
        result = static_unique_pointer_cast<CommandResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

/// Tell program to hangup
std::unique_ptr<TerminateResponse> Requestor::issueTerminateCommand() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<TerminateResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    TerminateRequest requestMessage;
    auto message = pImpl->request(requestMessage);
    if (message != nullptr)
    {
        if (message->getMessageType() == failureMessage.getMessageType())
        {
            failureMessage.fromMessage(message->toMessage());
            throw std::runtime_error(
                "Failed to issue terminate command.  Failed with: "
               + failureMessage.getDetails());
        }
        result = static_unique_pointer_cast<TerminateResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

/// Disconnect
void Requestor::disconnect()
{
    pImpl->disconnect();
}

