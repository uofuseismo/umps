#include <string>
#include <filesystem>
#include "umps/services/command/localRequestor.hpp"
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Services::Command;

class LocalRequestor::LocalRequestorImpl
{
public:
    /// @brief Constructor
    LocalRequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                       std::shared_ptr<UMPS::Logging::ILog> logger) :
        mRequest(std::make_unique<UMPS::Messaging::RequestRouter::Request> (context, logger))
    {
        // Make the message types
        std::unique_ptr<UMPS::MessageFormats::IMessage>
            availableCommandsResponseMessage
               = std::make_unique<AvailableCommandsResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> commandsResponse
            = std::make_unique<CommandResponse> ();
        mMessageFormats.add(availableCommandsResponseMessage);
        mMessageFormats.add(commandsResponse);
    }
    /// @brief Disconnect
    void disconnect()
    {
        mRequest->disconnect();
    }
    UMPS::MessageFormats::Messages mMessageFormats;
    std::unique_ptr<UMPS::Messaging::RequestRouter::Request> mRequest{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    LocalRequestorOptions mLocalRequestorOptions;
    UMPS::Messaging::RequestRouter::RequestOptions mRequestOptions;
    std::filesystem::path mIPCFileName;
    std::string mIPCName; // e.g., ipc://mIPCFileName
    LocalRequestorOptions mOptions;
};

/// C'tor
LocalRequestor::LocalRequestor() :
    pImpl(std::make_unique<LocalRequestorImpl> (nullptr, nullptr))
{
}

LocalRequestor::LocalRequestor(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<LocalRequestorImpl> (context, nullptr))
{
}

LocalRequestor::LocalRequestor(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<LocalRequestorImpl> (nullptr, logger))
{
}

LocalRequestor::LocalRequestor(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<LocalRequestorImpl> (context, logger))
{
}

/// Move c'tor
LocalRequestor::LocalRequestor(LocalRequestor &&requestor) noexcept
{
    *this = std::move(requestor);
}

/// Move assignment
LocalRequestor& LocalRequestor::operator=(LocalRequestor &&requestor) noexcept
{
    if (&requestor == this){return *this;}
    pImpl = std::move(requestor.pImpl);
    return *this;
}

/// Destructor
LocalRequestor::~LocalRequestor() = default;

/// Initialize
void LocalRequestor::initialize(const LocalRequestorOptions &options)
{
    if (!options.haveModuleName())
    {
        throw std::invalid_argument("Module name not set");
    }
    pImpl->disconnect();
    pImpl->mIPCFileName = options.getIPCFileName();
    if (!std::filesystem::exists(pImpl->mIPCFileName))
    {
        throw std::runtime_error("IPC file " + pImpl->mIPCFileName.string()
                               + " does not exist");
    }
    UMPS::Messaging::RequestRouter::RequestOptions requestOptions;
    pImpl->mIPCName = "ipc://" + pImpl->mIPCFileName.string();
    requestOptions.setAddress(pImpl->mIPCName);
    requestOptions.setTimeOut(options.getReceiveTimeOut());

    auto messageFormats = pImpl->mMessageFormats.get();
    for (auto &messageFormat : messageFormats)
    {
        requestOptions.addMessageFormat(messageFormat.second);
    }
    pImpl->mRequest->initialize(requestOptions);
    pImpl->mLocalRequestorOptions = options;
    pImpl->mRequestOptions = requestOptions;
}

/*
    auto timeOut = options.getReceiveTimeOut();
    pImpl->mLogger->debug("Connecting to: " + pImpl->mIPCName);
    pImpl->mRequest->set(zmq::sockopt::rcvtimeo,
                         static_cast<int> (timeOut.count()));
    pImpl->mRequest->connect(pImpl->mIPCName);
    pImpl->mIPCName = pImpl->mRequest->get(zmq::sockopt::last_endpoint);
    pImpl->mConnected = true;
}
*/

/// Initialized?
bool LocalRequestor::isInitialized() const noexcept
{
    return pImpl->mRequest->isInitialized();
}

/// Commands
std::unique_ptr<AvailableCommandsResponse> LocalRequestor::getCommands() const
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

std::unique_ptr<CommandResponse> LocalRequestor::issueCommand(
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

void LocalRequestor::disconnect()
{
    pImpl->disconnect();
}
