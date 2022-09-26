#include <string>
#include <filesystem>
#include "umps/services/command/localRequestor.hpp"
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/services/command/remoteRequestor.hpp"
#include "umps/services/command/remoteRequestorOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/terminateRequest.hpp"
#include "umps/services/command/terminateResponse.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/context.hpp"

using namespace UMPS::Services::Command;

class LocalRequestor::LocalRequestorImpl
{
public:
    /// @brief Constructor
    LocalRequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                       std::shared_ptr<UMPS::Logging::ILog> logger) :
        mRequestor(std::make_unique<RemoteRequestor> (context, logger))
    {
    }
    std::unique_ptr<RemoteRequestor> mRequestor{nullptr};
    LocalRequestorOptions mLocalRequestorOptions;
    std::filesystem::path mIPCFileName;
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
    pImpl->mIPCFileName = options.getIPCFileName();
    if (!std::filesystem::exists(pImpl->mIPCFileName))
    {
        throw std::runtime_error("IPC file " + pImpl->mIPCFileName.string()
                               + " does not exist");
    }
    auto requestOptions = options.getOptions();
    pImpl->mRequestor->initialize(requestOptions);
    pImpl->mLocalRequestorOptions = options;
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
    return pImpl->mRequestor->isInitialized();
    //return pImpl->mRequest->isInitialized();
}

/// Commands
std::unique_ptr<AvailableCommandsResponse> LocalRequestor::getCommands() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    return pImpl->mRequestor->getCommands();
}

std::unique_ptr<CommandResponse> LocalRequestor::issueCommand(
    const CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    return pImpl->mRequestor->issueCommand(request);
}

std::unique_ptr<TerminateResponse> LocalRequestor::issueTerminateCommand() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    return pImpl->mRequestor->issueTerminateCommand();
}

void LocalRequestor::disconnect()
{
    pImpl->mRequestor->disconnect();
}

