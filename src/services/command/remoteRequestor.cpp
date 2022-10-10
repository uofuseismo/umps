#include <string>
#include <filesystem>
#include "umps/services/command/remoteRequestor.hpp"
#include "umps/services/command/remoteRequestorOptions.hpp"
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/availableModulesRequest.hpp"
#include "umps/services/command/availableModulesResponse.hpp"
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

using namespace UMPS::Services::Command;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class RemoteRequestor::RemoteRequestorImpl
{
public:
    /// @brief Constructor
    RemoteRequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
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
               = std::make_unique<AvailableCommandsResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage>
            availableModulesResponse
               = std::make_unique<AvailableModulesResponse> (); 
        std::unique_ptr<UMPS::MessageFormats::IMessage> commandsResponse
            = std::make_unique<CommandResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> terminateResponse
            = std::make_unique<TerminateResponse> ();
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
    RemoteRequestorOptions mRemoteRequestorOptions;
    UMPS::Messaging::RequestRouter::RequestOptions mRequestOptions;
};

/// C'tor
RemoteRequestor::RemoteRequestor() :
    pImpl(std::make_unique<RemoteRequestorImpl> (nullptr, nullptr))
{
}

RemoteRequestor::RemoteRequestor(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RemoteRequestorImpl> (context, nullptr))
{
}

RemoteRequestor::RemoteRequestor(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteRequestorImpl> (nullptr, logger))
{
}

RemoteRequestor::RemoteRequestor(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteRequestorImpl> (context, logger))
{
}

/// Move c'tor
RemoteRequestor::RemoteRequestor(RemoteRequestor &&requestor) noexcept
{
    *this = std::move(requestor);
}

/// Move assignment
RemoteRequestor& 
RemoteRequestor::operator=(RemoteRequestor &&requestor) noexcept
{
    if (&requestor == this){return *this;}
    pImpl = std::move(requestor.pImpl);
    return *this;
}

/// Destructor
RemoteRequestor::~RemoteRequestor() = default;

/// Initialize
void RemoteRequestor::initialize(const RemoteRequestorOptions &options)
{
    pImpl->disconnect();
    auto requestOptions = options.getOptions();
    auto messageFormats = pImpl->mMessageFormats.get();
    for (auto &messageFormat : messageFormats)
    {
        requestOptions.addMessageFormat(messageFormat.second);
    }
    pImpl->mRequest->initialize(requestOptions);
    pImpl->mRemoteRequestorOptions = options;
    pImpl->mRequestOptions = requestOptions;
}

/// Initialized?
bool RemoteRequestor::isInitialized() const noexcept
{
    return pImpl->mRequest->isInitialized();
}

/// Available modules
std::unique_ptr<AvailableModulesResponse>
    RemoteRequestor::getAvailableModules() const
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
std::unique_ptr<AvailableCommandsResponse> RemoteRequestor::getCommands() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<AvailableCommandsResponse> result{nullptr};
    AvailableCommandsRequest requestMessage;
    auto message = pImpl->mRequest->request(requestMessage);
    if (message != nullptr)
    {
        result = static_unique_pointer_cast<AvailableCommandsResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

std::unique_ptr<CommandResponse> RemoteRequestor::issueCommand(
    const CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    std::unique_ptr<CommandResponse> result{nullptr};
    auto message = pImpl->mRequest->request(request);
    if (message != nullptr)
    {
        result = static_unique_pointer_cast<CommandResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

std::unique_ptr<TerminateResponse>
    RemoteRequestor::issueTerminateCommand() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<TerminateResponse> result{nullptr};
    TerminateRequest requestMessage;
    auto message = pImpl->mRequest->request(requestMessage);
    if (message != nullptr)
    {
        result = static_unique_pointer_cast<TerminateResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

void RemoteRequestor::disconnect()
{
    pImpl->disconnect();
}

