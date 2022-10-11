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
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UCommand = UMPS::Services::Command;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class Requestor::RequestorImpl
{
public:
    /// @brief Constructor
    RequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                       std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        else
        {
            mLogger = logger; 
        }
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> (1);
        }
        else
        {
            mContext = context;
        }
        mRequest
            = std::make_unique<URequestRouter::Request> (mContext, mLogger);
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
    /// @brief Disconnect
    void disconnect()
    {
        mRequest->disconnect();
    }
    UMPS::MessageFormats::Messages mMessageFormats;
    std::unique_ptr<URequestRouter::Request> mRequest{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
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
    pImpl->mRequest->initialize(requestOptions);
    pImpl->mRequestorOptions = options;
    pImpl->mRequestOptions = requestOptions;
}

/// Initialized?
bool Requestor::isInitialized() const noexcept
{
    return pImpl->mRequest->isInitialized();
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
    auto message = pImpl->mRequest->request(requestMessage);
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
    Requestor::getCommands() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<UCommand::AvailableCommandsResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    UCommand::AvailableCommandsRequest requestMessage;
    auto message = pImpl->mRequest->request(requestMessage);
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
    Requestor::issueCommand(const UCommand::CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    std::unique_ptr<UCommand::CommandResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    auto message = pImpl->mRequest->request(request);
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
    Requestor::issueTerminateCommand() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<UCommand::TerminateResponse> result{nullptr};
    UMPS::MessageFormats::Failure failureMessage;
    UCommand::TerminateRequest requestMessage;
    auto message = pImpl->mRequest->request(requestMessage);
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

