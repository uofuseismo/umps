#include <iostream>
#include <string>
#include "umps/broadcasts/dataPacket/broadcast.hpp"
#include "umps/broadcasts/dataPacket/parameters.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/messaging/publisherSubscriber/proxyOptions.hpp"
#include "umps/messaging/publisherSubscriber/proxy.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Broadcasts::DataPacket;

namespace UCI = UMPS::Services::ConnectionInformation;

class Broadcast::BroadcastImpl
{
public:
    /// Constructors
    BroadcastImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mProxy(mLogger)
    {   
    }
///private:
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    Parameters mParameters;
    UMPS::Messaging::PublisherSubscriber::Proxy mProxy;
    UCI::Details mConnectionDetails;
    const std::string mName = Parameters::getName();
    bool mInitialized = false;
};

/// C'tor
Broadcast::Broadcast() :
    pImpl(std::make_unique<BroadcastImpl> ())
{
}

/// Destructor
Broadcast::~Broadcast() = default;

/// Initialize the proxy
void Broadcast::initialize(const Parameters &parameters)
{
    if (!parameters.haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    if (!parameters.haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    stop();
    // Set the proxy options
    UMPS::Messaging::PublisherSubscriber::ProxyOptions options;
    options.setFrontendAddress(parameters.getFrontendAddress());
    options.setBackendAddress(parameters.getBackendAddress());
    options.setFrontendHighWaterMark(parameters.getFrontendHighWaterMark());
    options.setBackendHighWaterMark(parameters.getBackendHighWaterMark());
    options.setTopic(getName());
    pImpl->mProxy.initialize(options);
    // Figure out the connection details
    UCI::SocketDetails::XSubscriber xSub;
    UCI::SocketDetails::XPublisher xPub;
    xSub.setAddress(pImpl->mProxy.getFrontendAddress());
    xPub.setAddress(pImpl->mProxy.getBackendAddress());
    UCI::SocketDetails::Proxy proxyDetails;
    proxyDetails.setSocketPair(std::pair(xSub, xPub));
    pImpl->mConnectionDetails.setName(getName());
    pImpl->mConnectionDetails.setSocketDetails(proxyDetails);
    pImpl->mConnectionDetails.setConnectionType(UCI::ConnectionType::BROADCAST);
    pImpl->mConnectionDetails.setSecurityLevel(
        pImpl->mProxy.getSecurityLevel());
    // Save
    pImpl->mParameters = parameters;
    pImpl->mInitialized = true;
}
/*
void proxy()
{
    ProxyOptions options;
    UAuth::ZAPOptions zapOptions;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(100);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(200);
    options.setTopic(topic);
    options.setZAPOptions(zapOptions);
    // Make a logger
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Initialize the server
    UMPS::Messaging::PublisherSubscriber::Proxy proxy(loggerPtr);
    proxy.initialize(options);
    // A thread runs the proxy
    std::thread t1(&UMPS::Messaging::PublisherSubscriber::Proxy::start,
                   &proxy);
    // Main thread waits...
    std::this_thread::sleep_for(std::chrono::seconds(3));
    /// Main thread tells proxy to stop
    proxy.stop();
    t1.join();
}
*/

/// Start the service
void Broadcast::start()
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    pImpl->mLogger->info("Beginning " + getName() + " broadcast...");
    pImpl->mProxy.start();
    pImpl->mLogger->info("Exited " + getName() + " broadcast proxy");
}

/// Is the proxy running?
bool Broadcast::isRunning() const noexcept
{
    return pImpl->mProxy.isRunning();
}

/// Stop the service
void Broadcast::stop()
{
    pImpl->mProxy.stop();
}

/// Initialized?
bool Broadcast::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Gets the broadcast name
std::string Broadcast::getName() const
{
    return pImpl->mName;
}

/// Connection details
UCI::Details Broadcast::getConnectionDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Broadcast " + getName() + " not initialized");
    }
    return pImpl->mConnectionDetails;
}
