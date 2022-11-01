#include <iostream>
#include <vector>
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/standardOut.hpp"
#include "umps/messageFormats/staticUniquePointerCast.hpp"
#include "private/isEmpty.hpp"

namespace URequestRouter = UMPS::Messaging::RequestRouter;
namespace UAuth = UMPS::Authentication;
namespace UMF = UMPS::MessageFormats;
using namespace UMPS::Services::ConnectionInformation;

class Requestor::RequestorImpl
{
public:
    /// Constructors
    RequestorImpl() = delete;
    RequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StandardOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mRequestor = std::make_unique<URequestRouter::Request>
                     (context, mLogger);
    }
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::unique_ptr<URequestRouter::Request> mRequestor;
    RequestorOptions mOptions;
};

/// C'tor
Requestor::Requestor() :
    pImpl(std::make_unique<RequestorImpl> (nullptr, nullptr))
{
}

/// C'tor with logger
Requestor::Requestor(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestorImpl> (nullptr,  logger))
{
}

/// C'tor with context
Requestor::Requestor(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RequestorImpl> (context, nullptr))
{
}

/// C'tor with context and logger
Requestor::Requestor(std::shared_ptr<UMPS::Messaging::Context> &context,
                     std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestorImpl> (context, logger))
{
}

/// Move c'tor
Requestor::Requestor(Requestor &&request) noexcept
{
    *this = std::move(request);
}

/// Destructor
Requestor::~Requestor() = default;

/// Move assignment
Requestor& Requestor::operator=(Requestor &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Initialize
void Requestor::initialize(const RequestorOptions &requestOptions)
{
    auto options = requestOptions.getRequestOptions();
    if (!options.haveAddress())
    {
        throw std::invalid_argument("End point not set");
    } 
    pImpl->mRequestor->initialize(options);
    pImpl->mOptions = requestOptions;
}

/// Initialized?
bool Requestor::isInitialized() const noexcept
{
    return pImpl->mRequestor->isInitialized();
} 

/// Get all connection details
std::vector<Details> Requestor::getAllConnectionDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor client not initialized");
    }
    std::vector<Details> result;
    AvailableConnectionsRequest requestMessage;
    auto message = pImpl->mRequestor->request(requestMessage);
    if (message != nullptr)
    {
        auto detailsMessage
            = UMF::static_unique_pointer_cast<AvailableConnectionsResponse>
              (std::move(message));
        result = detailsMessage->getDetails();
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

/// Get proxyBroadcast frontend 
SocketDetails::XSubscriber
Requestor::getProxyBroadcastFrontendDetails(const std::string &name) const
{
    if (isEmpty(name))
    {
        throw std::invalid_argument("Proxy broadcast name is empty");
    }
    auto connectionDetails = getAllConnectionDetails();
    for (const auto &connectionDetail : connectionDetails)
    {
        if (connectionDetail.getName() == name)
        {
            auto proxySocketDetails = connectionDetail.getProxySocketDetails();
            return proxySocketDetails.getXSubscriberFrontend();
        }
    }
    throw std::runtime_error("No details found for: " + name);
}

/// Get proxyBroadcast frontend 
SocketDetails::XPublisher
Requestor::getProxyBroadcastBackendDetails(const std::string &name) const
{
    if (isEmpty(name))
    {
        throw std::invalid_argument("Proxy broadcast name is empty");
    }
    auto connectionDetails = getAllConnectionDetails();
    for (const auto &connectionDetail : connectionDetails)
    {
        if (connectionDetail.getName() == name)
        {
            auto proxySocketDetails = connectionDetail.getProxySocketDetails();
            return proxySocketDetails.getXPublisherBackend();
        }
    }
    throw std::runtime_error("No details found for: " + name);
}

/// Get proxyService frontend
SocketDetails::Router
Requestor::getProxyServiceFrontendDetails(const std::string &name) const
{
    if (isEmpty(name))
    {
        throw std::invalid_argument("Proxy service name is empty");
    }
    auto connectionDetails = getAllConnectionDetails();
    for (const auto &connectionDetail : connectionDetails)
    {
        if (connectionDetail.getName() == name)
        {
            auto proxySocketDetails = connectionDetail.getProxySocketDetails();
            return proxySocketDetails.getRouterFrontend();
        }
    }
    throw std::runtime_error("No details found for: " + name);
}

/// Get proxyService backend
SocketDetails::Dealer
Requestor::getProxyServiceBackendDetails(const std::string &name) const
{
    if (isEmpty(name))
    {
        throw std::invalid_argument("Proxy service name is empty");
    }
    auto connectionDetails = getAllConnectionDetails();
    for (const auto &connectionDetail : connectionDetails)
    {
        if (connectionDetail.getName() == name)
        {
            auto proxySocketDetails = connectionDetail.getProxySocketDetails();
            return proxySocketDetails.getDealerBackend();
        }
    }
    throw std::runtime_error("No details found for: " + name);
}

/// The requestor options
RequestorOptions Requestor::getRequestorOptions() const
{
    if (!isInitialized()){throw std::runtime_error("Requestor not initialized");}
    return pImpl->mOptions;
}

/// My ZAP information
UAuth::ZAPOptions Requestor::getZAPOptions() const
{
    return getRequestorOptions().getRequestOptions().getZAPOptions();
}

/// Disconnect
void Requestor::disconnect()
{
    pImpl->mRequestor->disconnect();
}

/// Create from ini file
std::unique_ptr<Requestor>
UMPS::Services::ConnectionInformation::createRequestor(
    const std::string &iniFile,
    const std::string &section,
    std::shared_ptr<UMPS::Messaging::Context> context,
    std::shared_ptr<UMPS::Logging::ILog> logger)
{
    // Load the essential operator connection information from an ini file
    RequestorOptions options;
    options.parseInitializationFile(iniFile, section);
    auto requestor = std::make_unique<Requestor> (context, logger);
    requestor->initialize(options);
    return requestor;
}

