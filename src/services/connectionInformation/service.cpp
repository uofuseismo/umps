#include <iostream>
#include <string>
#include <chrono>
#include <functional>
#include "umps/services/service.hpp"
#include "umps/broadcasts/broadcast.hpp"
#include "umps/services/connectionInformation/service.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
//#include "umps/services/connectionInformation/response.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/logging/stdout.hpp"

class Parameters
{
std::string getClientAccessAddress();
};

using namespace UMPS::Services::ConnectionInformation;

class Service::ServiceImpl
{
public:
    /// Constructors
    ServiceImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mRouter(mLogger)
    {
    }
    explicit ServiceImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger),
        mRouter(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared< UMPS::Logging::StdOut> ();
        }
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
                mLogger->error("Broadcasts request serialization failed with: "
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
        }
        return response;
    }
///private:
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr; 
    ConnectionInformation::Details mConnectionDetails;
    std::map<std::string, ConnectionInformation::Details> mConnections;
    UMPS::Messaging::RequestRouter::Router mRouter;
    const std::string mName = "ConnectionInformation";
    bool mInitialized = false;
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> ())
{
}

/// Destructor
Service::~Service() = default;

/// Initialize
void Service::initialize(const Parameters &parameters)
{
    stop(); // Ensure the service is stopped
    // Clear out the old services and broadcasts
    pImpl->mConnections.clear();
    // Initialize the socket - Step 1: Initialize options
    Messaging::RequestRouter::RouterOptions routerOptions;
//    auto clientAccessAddress = parameters.getClientAccessAddress();
    std::string clientAccessAddress;
    routerOptions.setEndPoint(clientAccessAddress);
    routerOptions.setCallback(std::bind(&ServiceImpl::callback,
                                        &*this->pImpl,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
    // Add the message types
    std::unique_ptr<UMPS::MessageFormats::IMessage> requestType
        = std::make_unique<AvailableConnectionsRequest> (); 
    routerOptions.addMessageFormat(requestType);
    pImpl->mRouter.initialize(routerOptions); 
    // Create the connection details
    ConnectionInformation::SocketDetails::Router socketDetails;
    socketDetails.setAddress(pImpl->mRouter.getConnectionString());
    pImpl->mConnectionDetails.setName(pImpl->mName);
    pImpl->mConnectionDetails.setSocketDetails(socketDetails);
    // Add myself
    pImpl->mConnections.insert(std::pair(pImpl->mName,
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
    return pImpl->mRouter.isRunning();
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
    pImpl->mRouter.stop();
}

/// Runs the service
void Service::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Service not initialized");
    }
    pImpl->mLogger->debug("Beginning service " + pImpl->mName + "...");
    pImpl->mRouter.start(); // This should hang until the service is stopped
    pImpl->mLogger->debug("Thread exiting service " + pImpl->mName);
}

