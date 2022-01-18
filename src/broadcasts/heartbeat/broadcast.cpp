#include <iostream>
#include <string>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include "umps/broadcasts/heartbeat/broadcast.hpp"
#include "umps/broadcasts/heartbeat/parameters.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxy.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/service.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Broadcasts::Heartbeat;
namespace UAuth = UMPS::Authentication;
namespace UXPubXSub = UMPS::Messaging::XPublisherXSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;

class Broadcast::BroadcastImpl
{
public:
    /// Constructors
    BroadcastImpl() = delete;
    BroadcastImpl(std::shared_ptr<zmq::context_t> context,
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
        mProxy = std::make_unique<UMPS::Messaging::XPublisherXSubscriber::Proxy>
                 (mContext, mLogger);
        mAuthenticatorService = std::make_unique<UAuth::Service>
                                (mContext, mLogger, mAuthenticator);
    }
    /// Stops the proxy and authenticator and joins threads
    void stop()
    {
        if (mProxy->isRunning()){mProxy->stop();}
        if (mAuthenticatorService->isRunning()){mAuthenticatorService->stop();}
        if (mProxyThread.joinable()){mProxyThread.join();}
        if (mAuthenticatorThread.joinable()){mAuthenticatorThread.join();}
    }
    /// Starts the proxy and authenticator and creates threads
    void start()
    {
        stop();
#ifndef NDEBUG
        assert(mProxy->isInitialized());
#endif
        mProxyThread = std::thread(&UXPubXSub::Proxy::start,
                                   &*mProxy);
        mAuthenticatorThread = std::thread(&UAuth::Service::start,
                                           &*mAuthenticatorService);
    }
/*
    explicit BroadcastImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
       mLogger(logger),
       mProxy(mLogger)
    {
       if (mLogger == nullptr)
       {
           mLogger = std::make_shared<UMPS::Logging::StdOut> ();
       }
    }
*/
///private:
    std::shared_ptr<zmq::context_t> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<UXPubXSub::Proxy> mProxy{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    Parameters mParameters;
    UCI::Details mConnectionDetails;
    std::thread mProxyThread;
    std::thread mAuthenticatorThread;
    const std::string mName = Parameters::getName();
    bool mInitialized = false;
};

/// C'tor
Broadcast::Broadcast() :
    pImpl(std::make_unique<BroadcastImpl> (nullptr, nullptr, nullptr))
{
}

Broadcast::Broadcast(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<BroadcastImpl> (nullptr, logger, nullptr))
{
}

Broadcast::Broadcast(std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<BroadcastImpl> (nullptr, nullptr, authenticator))
{
}

Broadcast::Broadcast(std::shared_ptr<UMPS::Logging::ILog> &logger,
                     std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<BroadcastImpl> (nullptr, logger, authenticator))
{
}

/*
/// Move c'tor
Broadcast::Broadcast(Broadcast &&broadcast) noexcept
{
    *this = std::move(broadcast);
}

/// Move assignment
Broadcast& Broadcast::operator=(Broadcast &&broadcast) noexcept
{
    if (&broadcast == this){return *this;}
    pImpl = std::move(broadcast.pImpl);
    return *this;
}
*/

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
    UMPS::Messaging::XPublisherXSubscriber::ProxyOptions options;
    options.setFrontendAddress(parameters.getFrontendAddress());
    options.setBackendAddress(parameters.getBackendAddress());
    options.setFrontendHighWaterMark(parameters.getFrontendHighWaterMark());
    options.setBackendHighWaterMark(parameters.getBackendHighWaterMark());
    options.setTopic(getName());
    options.setZAPOptions(parameters.getZAPOptions());
    pImpl->mProxy->initialize(options);
    // Figure out the connection details
    pImpl->mConnectionDetails.setName(getName());
    pImpl->mConnectionDetails.setSocketDetails(
        pImpl->mProxy->getSocketDetails());
    pImpl->mConnectionDetails.setConnectionType(UCI::ConnectionType::BROADCAST);
    pImpl->mConnectionDetails.setSecurityLevel(
        pImpl->mProxy->getSecurityLevel());
    // Save
    pImpl->mParameters = parameters;
    pImpl->mInitialized = true;
}

/// Start the service
void Broadcast::start()
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    pImpl->mLogger->info("Beginning " + getName() + " broadcast...");
pImpl->start();
    //pImpl->mProxy->start();
    pImpl->mLogger->info("Exited " + getName() + " broadcast proxy");
}

/// Is the proxy running?
bool Broadcast::isRunning() const noexcept
{
    return pImpl->mProxy->isRunning();
}

/// Stop the service
void Broadcast::stop()
{
//    pImpl->mProxy->stop();
pImpl->stop();
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

/// Create an instance of this clsas
std::unique_ptr<UMPS::Broadcasts::IBroadcast>
    Broadcast::createInstance() const noexcept
{
    std::unique_ptr<UMPS::Broadcasts::IBroadcast> result
        = std::make_unique<Broadcast> ();
    return result;
}
