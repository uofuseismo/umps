#include <iostream>
#include <string>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include "umps/proxyBroadcasts/dataPacket/proxy.hpp"
#include "umps/proxyBroadcasts/dataPacket/proxyOptions.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxy.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/authentication/service.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::ProxyBroadcasts::DataPacket;
namespace UAuth = UMPS::Authentication;
namespace UXPubXSub = UMPS::Messaging::XPublisherXSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;

class Proxy::ProxyImpl
{
public:
    /// Constructors
    /*
    ProxyImpl() = delete;
    ProxyImpl(std::shared_ptr<zmq::context_t> context,
              std::shared_ptr<UMPS::Logging::ILog> logger,
              std::shared_ptr<UAuth::IAuthenticator> authenticator, int)
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
        mProxy = std::make_unique<UXPubXSub::Proxy> (mContext, mLogger);
        mAuthenticatorService = std::make_unique<UAuth::Service>
                                (mContext, mLogger, mAuthenticator);
    }
    */
    ProxyImpl(std::shared_ptr<UMPS::Messaging::Context> context,
              std::shared_ptr<UMPS::Logging::ILog> logger,
              std::shared_ptr<UAuth::IAuthenticator> authenticator)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> (1);
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
        mProxy = std::make_unique<UXPubXSub::Proxy> (mInternalContext, mLogger);
        mAuthenticatorService = std::make_unique<UAuth::Service>
                                (mInternalContext, mLogger, mAuthenticator);
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
    /// Destructor
    ~ProxyImpl()
    {
        stop();
    }
///private:
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::shared_ptr<UXPubXSub::Proxy> mProxy{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    ProxyOptions mOptions;
    UCI::Details mConnectionDetails;
    std::thread mProxyThread;
    std::thread mAuthenticatorThread;
    bool mInitialized = false;
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> (nullptr, nullptr, nullptr))
{
}

/*
Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr, nullptr))
{
}
*/

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
             std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger, authenticator))
{
}

/*
Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
             std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr, authenticator))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
             std::shared_ptr<UMPS::Logging::ILog> &logger,
             std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ProxyImpl> (context, logger, authenticator))
{
}

/// Move c'tor
Proxy::Proxy(Proxy &&proxy) noexcept
{
    *this = std::move(proxy);
}

/// Move assignment
Proxy& Proxy::operator=(Proxy &&proxy) noexcept
{
    if (&broadcast == this){return *this;}
    pImpl = std::move(broadcast.pImpl);
    return *this;
}
*/

/// Destructor
Proxy::~Proxy() = default;

/// Initialize the proxy
void Proxy::initialize(const ProxyOptions &parameters)
{
    if (!parameters.haveName())
    {
        throw std::invalid_argument("Proxy name not set");
    }
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
    pImpl->mOptions = parameters;
    UMPS::Messaging::XPublisherXSubscriber::ProxyOptions options;
    options.setFrontendAddress(pImpl->mOptions.getFrontendAddress());
    options.setBackendAddress(pImpl->mOptions.getBackendAddress());
    options.setFrontendHighWaterMark(
        pImpl->mOptions.getFrontendHighWaterMark());
    options.setBackendHighWaterMark(
        pImpl->mOptions.getBackendHighWaterMark());
    options.setZAPOptions(pImpl->mOptions.getZAPOptions());
    pImpl->mProxy->initialize(options);
    // Figure out the connection details
    pImpl->mConnectionDetails.setName(pImpl->mOptions.getName());
    pImpl->mConnectionDetails.setSocketDetails(
        pImpl->mProxy->getSocketDetails());
    pImpl->mConnectionDetails.setConnectionType(UCI::ConnectionType::BROADCAST);
    pImpl->mConnectionDetails.setSecurityLevel(
        pImpl->mProxy->getSecurityLevel());
    pImpl->mInitialized = true;
}

/// Start the service
void Proxy::start()
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    pImpl->mLogger->info("Beginning " + getName() + " proxy broadcast...");
    pImpl->start();
}

/// Is the proxy running?
bool Proxy::isRunning() const noexcept
{
    return pImpl->mProxy->isRunning();
}

/// Stop the service
void Proxy::stop()
{
    pImpl->stop();
}

/// Initialized?
bool Proxy::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Gets the proxy name
std::string Proxy::getName() const
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    return pImpl->mOptions.getName();
}

/// Connection details
UCI::Details Proxy::getConnectionDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("ProxyBroadcast " + getName()
                               + " not initialized");
    }
    return pImpl->mConnectionDetails;
}

/*
/// Create an instance of this class
std::unique_ptr<UMPS::ProxyBroadcasts::IProxy>
    Proxy::createInstance() const noexcept
{
    std::unique_ptr<UMPS::ProxyBroadcasts::IProxy> result
        = std::make_unique<Proxy> ();
    return result;
}
*/
