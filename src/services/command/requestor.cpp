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
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/context.hpp"

using namespace UMPS::Services::Command;

class Requestor::RequestorImpl
{
public:
    /// @brief Constructor
    RequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                       std::shared_ptr<UMPS::Logging::ILog> logger) :
        mRequestor(std::make_unique<ProxyServices::Command::Requestor>
                   (context, logger))
    {
    }
    std::unique_ptr<UMPS::ProxyServices::Command::Requestor>
        mRequestor{nullptr};
    RequestorOptions mRequestorOptions;
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
    auto requestOptions = options.getOptions();
    pImpl->mRequestor->initialize(requestOptions);
    pImpl->mRequestorOptions = options;
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
bool Requestor::isInitialized() const noexcept
{
    return pImpl->mRequestor->isInitialized();
    //return pImpl->mRequest->isInitialized();
}

/// Commands
std::unique_ptr<AvailableCommandsResponse> Requestor::getCommands() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    return pImpl->mRequestor->getCommands();
}

std::unique_ptr<CommandResponse> Requestor::issueCommand(
    const CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    return pImpl->mRequestor->issueCommand(request);
}

std::unique_ptr<TerminateResponse> Requestor::issueTerminateCommand() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    return pImpl->mRequestor->issueTerminateCommand();
}

void Requestor::disconnect()
{
    pImpl->mRequestor->disconnect();
}

