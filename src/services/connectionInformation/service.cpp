#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <functional>
#include <zmq.hpp>
#include "umps/services/service.hpp"
#include "umps/broadcasts/broadcast.hpp"
#include "umps/proxyBroadcasts/proxy.hpp"
#include "umps/proxyServices/proxy.hpp"
#include "umps/services/connectionInformation/service.hpp"
#include "umps/services/connectionInformation/parameters.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
//#include "umps/services/connectionInformation/response.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/authentication/service.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace URequestRouter = UMPS::Messaging::RequestRouter;
namespace UAuth = UMPS::Authentication;

class Service::ServiceImpl
{
public:
    /// Constructors
    ServiceImpl() = delete;
    ServiceImpl(std::shared_ptr<zmq::context_t> context,
                std::shared_ptr<UMPS::Logging::ILog> logger,
                std::shared_ptr<UAuth::IAuthenticator> authenticator)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
        }
        else
        {
            mContext = context;
        }
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
        if (authenticator == nullptr)
        {
            mAuthenticator = std::make_shared<UAuth::Grasslands> (mLogger);
        } 
        else
        {
            mAuthenticator = authenticator;
        }
        mRouter = std::make_unique<URequestRouter::Router> (mContext, mLogger);
        mAuthenticatorService
            = std::make_unique<UAuth::Service>
              (mContext, mLogger, mAuthenticator);
    }
    /// The callback to handle connection requests
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *messageContents, const size_t length) noexcept
    {
        if (mLogger->getLevel() >= UMPS::Logging::Level::DEBUG)
        {
            mLogger->debug("ServiceImpl::callback: Message of type: "
                         + messageType 
                         + " with length: " + std::to_string(length)
                         + " bytes was received.  Processing...");
        }
        auto response = std::make_unique<AvailableConnectionsResponse> ();
        AvailableConnectionsRequest request;
        if (messageType == request.getMessageType())
        {
            // Unpack the request
            try
            {
                auto messagePtr = static_cast<const char *> (messageContents);
                request.fromMessage(messagePtr, length);
            }
            catch (const std::exception &e) 
            {
                mLogger->error("Request serialization failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
            }
        }
        else
        {
            mLogger->error("Received message type: " + messageType
                         + " but received can only process "
                         + request.getMessageType());
            response->setReturnCode(ReturnCode::INVALID_MESSAGE);
            return response;
        }
        // Response to the message
        try
        {
            std::vector<ConnectionInformation::Details> details;
            details.reserve(mConnections.size());
            for (const auto &connection : mConnections)
            {
                details.push_back(connection.second);
            }
            response->setDetails(details);
            response->setReturnCode(ReturnCode::SUCCESS);
        }
        catch (const std::exception &e)
        {
            mLogger->error("Failed to create message: "
                         + response->getMessageType() + " because " 
                         + e.what());
            response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
        } 
        return response;
    }
    /// Stops the proxy and authenticator and joins threads
    void stop()
    {   
        if (mRouter->isRunning()){mRouter->stop();}
        if (mAuthenticatorService->isRunning()){mAuthenticatorService->stop();}
        if (mProxyThread.joinable()){mProxyThread.join();}
        if (mAuthenticatorThread.joinable()){mAuthenticatorThread.join();}
    }
    /// Starts the proxy and authenticator and creates threads
    void start()
    {
        stop();
#ifndef NDEBUG
        assert(mRouter->isInitialized());
#endif
        mProxyThread = std::thread(&URequestRouter::Router::start,
                                   &*mRouter);
        mAuthenticatorThread = std::thread(&UAuth::Service::start,
                                           &*mAuthenticatorService);
    }
    /// Destructor
    ~ServiceImpl()
    {
        stop();
    }
///private:
    std::shared_ptr<zmq::context_t> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<URequestRouter::Router> mRouter{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    ConnectionInformation::Details mConnectionDetails;
    std::map<std::string, ConnectionInformation::Details> mConnections;
    UMPS::Messaging::RequestRouter::RouterOptions mRouterOptions;
    std::thread mProxyThread;
    std::thread mAuthenticatorThread;
    const std::string mName = Parameters::getName(); //"ConnectionInformation";
    bool mInitialized = false;
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> (nullptr, nullptr, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger, nullptr))
{
}

/*
Service::Service(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<ServiceImpl> (context, nullptr, nullptr))
{
}

Service::Service(std::shared_ptr<zmq::context_t> &context,
                std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (context, logger, nullptr))
{
}

Service::Service(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (context, nullptr, authenticator))
{
}
*/

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
                 std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger, authenticator))
{
}

/*
Service::Service(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger,
                 std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (context, logger, authenticator))
{
}
*/

/*
/// Move c'tor
Service::Service(Service &&service) noexcept
{
    *this = std::move(service);
}

/// Move assignment
Service& Service::operator=(Service &&service) noexcept
{
    if (&service == this){return *this;}
    pImpl = std::move(service.pImpl);
    return *this;
}
*/

/// Destructor
Service::~Service() = default;

/// Initialize
void Service::initialize(const Parameters &parameters)
{
    if (!parameters.haveClientAccessAddress())
    {
        throw std::invalid_argument("Client access address not set");
    }
    stop(); // Ensure the service is stopped
    // Clear out the old services and broadcasts
    pImpl->mConnections.clear();
    pImpl->mRouterOptions.clear();
    // Initialize the socket - Step 1: Initialize options
    auto clientAccessAddress = parameters.getClientAccessAddress();
    pImpl->mRouterOptions.setEndPoint(clientAccessAddress);
    pImpl->mRouterOptions.setCallback(std::bind(&ServiceImpl::callback,
                                                &*this->pImpl,
                                                std::placeholders::_1,
                                                std::placeholders::_2,
                                                std::placeholders::_3));
    pImpl->mRouterOptions.setZAPOptions(parameters.getZAPOptions());
    // Add the message types
    std::unique_ptr<UMPS::MessageFormats::IMessage> requestType
        = std::make_unique<AvailableConnectionsRequest> (); 
    //pImpl->mRouterOptions.addMessageFormat(requestType);
    pImpl->mRouter->initialize(pImpl->mRouterOptions); 
    // Create the connection details
    auto socketDetails = pImpl->mRouter->getSocketDetails();
    pImpl->mConnectionDetails.setName(getName());
    pImpl->mConnectionDetails.setSocketDetails(socketDetails);
    pImpl->mConnectionDetails.setConnectionType(ConnectionType::SERVICE);
    pImpl->mConnectionDetails.setSecurityLevel(
        socketDetails.getSecurityLevel());
    // Add myself
    pImpl->mConnections.insert(std::pair(getName(),
                                         pImpl->mConnectionDetails));
    // Done
    pImpl->mInitialized = true;
}

/// Initialized?
bool Service::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Gets the service name
std::string Service::getName() const
{
    return pImpl->mName;
}

/// Is the service running?
bool Service::isRunning() const noexcept
{
    return pImpl->mRouter->isRunning();
}

/// Connection details
UMPS::Services::ConnectionInformation::Details Service::getConnectionDetails() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mConnectionDetails; 
}


/// Gets the request address
std::string Service::getRequestAddress() const
{
    if (!isRunning()){throw std::runtime_error("Service is not running");}
    return pImpl->mConnectionDetails.getRouterSocketDetails().getAddress();
}

/// Stop the service
void Service::stop()
{
    //pImpl->mRouter->stop();
    pImpl->stop();
}

/// Runs the service
void Service::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Service not initialized");
    }
    pImpl->mLogger->debug("Beginning authenticator for service "
                        + getName() + "...");
    pImpl->start();
/*
    std::thread authThread(&UAuth::Service::start,
                           &*pImpl->mAuthenticatorService);
    pImpl->mLogger->debug("Beginning service " + getName() + "...");
    pImpl->mRouter->start(); // This should hang until the service is stopped
    pImpl->mLogger->debug("Thread stopping authenticator service on "
                        + getName());
    pImpl->mAuthenticatorService->stop();
    authThread.join();
    pImpl->mLogger->debug("Thread exiting service " + getName());
*/
}

/// Add (service) connection
void Service::addConnection(const UMPS::Services::IService &service)
{
    auto details = service.getConnectionDetails();
    addConnection(details);
}

/// Add (broadcast) connection
void Service::addConnection(const UMPS::Broadcasts::IBroadcast &broadcast)
{
    auto details = broadcast.getConnectionDetails();
    addConnection(details);
}

/// Add (proxy broadcast) connection
void Service::addConnection(const UMPS::ProxyBroadcasts::IProxy &proxy)
{
    auto details = proxy.getConnectionDetails();
    addConnection(details);
}

/// Add (proxy service) connection
void Service::addConnection(const UMPS::ProxyServices::IProxy &proxy)
{
    auto details = proxy.getConnectionDetails();
    addConnection(details);
}

/// Add connection
void Service::addConnection(const Details &details)
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    if (!details.haveName())
    {
        throw std::invalid_argument("Connection details name not set");
    }
    if (details.getSocketType() == SocketType::UNKNOWN)
    {
        throw std::invalid_argument("Socket type not set");
    }
    auto name = details.getName();
    if (haveConnection(name))
    {
        throw std::invalid_argument("Connection already set for " + name);
    }
    pImpl->mConnections.insert(std::pair(name, details));
}

/// Remove connection
void Service::removeConnection(const std::string &name)
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    if (!haveConnection(name))
    {
        throw std::runtime_error("Connection " + name + " does not exist");
    }
    auto idx = pImpl->mConnections.find(name);
    pImpl->mConnections.erase(idx);
}

/// Have service?
bool Service::haveConnection(const std::string &name) const noexcept
{
    return (pImpl->mConnections.find(name) != pImpl->mConnections.end());
}
