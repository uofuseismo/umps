#include <iostream>
#include <string>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include "umps/proxyBroadcasts/proxy.hpp"
#include "umps/proxyBroadcasts/proxyOptions.hpp"
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
#include "umps/logging/standardOut.hpp"

using namespace UMPS::ProxyBroadcasts;
namespace UAuth = UMPS::Authentication;
namespace UXPubXSub = UMPS::Messaging::XPublisherXSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;

class Proxy::ProxyImpl
{
public:
    /// Constructors
    ProxyImpl() = delete;
    ProxyImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
              const std::shared_ptr<UMPS::Logging::ILog> &logger,
              const std::shared_ptr<UAuth::IAuthenticator> &authenticator)
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
            mLogger = std::make_shared<UMPS::Logging::StandardOut> (); 
        }
        else
        {
            mLogger = logger;
        }
        mSymmetricAuthentication = true;
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
    ProxyImpl(const std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
              const std::shared_ptr<UMPS::Messaging::Context> &backendContext,
              const std::shared_ptr<UMPS::Logging::ILog> &logger,
              const std::shared_ptr<UAuth::IAuthenticator> &frontendAuthenticator,
              const std::shared_ptr<UAuth::IAuthenticator> &backendAuthenticator)
    {
        // Handle context
        if (frontendContext == nullptr)
        {
            mFrontendContext = std::make_shared<UMPS::Messaging::Context> (1);
        }
        else
        {
            mFrontendContext = frontendContext;
        }
        if (backendContext == nullptr)
        {
            mBackendContext = std::make_shared<UMPS::Messaging::Context> (1);
        }
        else
        {
            mBackendContext = backendContext;
        }
#ifndef NDEBUG
        assert(mFrontendContext != nullptr);
        assert(mBackendContext  != nullptr);
#endif
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StandardOut> (); 
        }
        else
        {
            mLogger = logger;
        }
        // Create authenticator
        mSymmetricAuthentication = false;
        if (frontendAuthenticator == nullptr)
        {
            mFrontendAuthenticator
                = std::make_shared<UAuth::Grasslands> (mLogger);
        }
        else
        {   
            mFrontendAuthenticator = frontendAuthenticator;
        }
        if (backendAuthenticator == nullptr)
        {
            mBackendAuthenticator 
                = std::make_shared<UAuth::Grasslands> (mLogger);
        }
        else
        {
            mBackendAuthenticator = backendAuthenticator;
        }
#ifndef NDEBUG
        assert(mFrontendAuthenticator != nullptr);
        assert(mBackendAuthenticator  != nullptr);
#endif
        mProxy = std::make_unique<UXPubXSub::Proxy> (mFrontendContext,
                                                     mBackendContext,
                                                     mLogger);
        mFrontendAuthenticatorService = std::make_unique<UAuth::Service>
                                        (mFrontendContext,
                                         mLogger,
                                         mFrontendAuthenticator);
        mBackendAuthenticatorService = std::make_unique<UAuth::Service>
                                        (mBackendContext, 
                                         mLogger,
                                         mBackendAuthenticator);
    }
    /// Stops the proxy and authenticator and joins threads
    void stop()
    {   
        if (mProxy->isRunning()){mProxy->stop();}
        if (mSymmetricAuthentication)
        {
            if (mAuthenticatorService->isRunning())
            {
                mAuthenticatorService->stop();
            }
        }
        else
        {
            if (mFrontendAuthenticatorService->isRunning())
            {
                mFrontendAuthenticatorService->stop();
            }
            if (mBackendAuthenticatorService->isRunning())
            {
                mBackendAuthenticatorService->stop();
            }
        }
        if (mProxyThread.joinable()){mProxyThread.join();}
        if (mAuthenticatorThread.joinable()){mAuthenticatorThread.join();}
        if (mFrontendAuthenticatorThread.joinable())
        {
            mFrontendAuthenticatorThread.join();
        }
        if (mBackendAuthenticatorThread.joinable())
        {
            mBackendAuthenticatorThread.join();
        } 
    }   
    /// Starts the proxy and authenticator and creates threads
    void start()
    {
        stop();
#ifndef NDEBUG
        assert(mProxy->isInitialized());
#endif
        if (mSymmetricAuthentication)
        {
            mAuthenticatorThread = std::thread(&UAuth::Service::start,
                                               &*mAuthenticatorService);
        }
        else
        {
            mFrontendAuthenticatorThread = std::thread(&UAuth::Service::start,
                                               &*mFrontendAuthenticatorService);
            mBackendAuthenticatorThread = std::thread(&UAuth::Service::start,
                                               &*mBackendAuthenticatorService);
        }
        // Give authenticators a chance to start then start proxy.  Otherwise,
        // a sneaky person can connect pre-authentication.
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
        mProxyThread = std::thread(&UXPubXSub::Proxy::start,
                                   &*mProxy);
    }
    /// Destructor
    ~ProxyImpl()
    {
        stop();
    }
///private:
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mFrontendContext{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mBackendContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::shared_ptr<UXPubXSub::Proxy> mProxy{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    std::unique_ptr<UAuth::Service> mFrontendAuthenticatorService{nullptr};
    std::unique_ptr<UAuth::Service> mBackendAuthenticatorService{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mFrontendAuthenticator{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mBackendAuthenticator{nullptr};
    UXPubXSub::ProxyOptions mProxyOptions;
    ProxyOptions mOptions;
    UCI::Details mConnectionDetails;
    std::thread mProxyThread;
    std::thread mAuthenticatorThread;
    std::thread mBackendAuthenticatorThread;
    std::thread mFrontendAuthenticatorThread;
    bool mInitialized{false};
    bool mSymmetricAuthentication{true};
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> (nullptr, nullptr, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
             std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger, authenticator))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
             std::shared_ptr<UAuth::IAuthenticator> &frontendAuthenticator,
             std::shared_ptr<UAuth::IAuthenticator> &backendAuthenticator) :
    pImpl(std::make_unique<ProxyImpl> (nullptr,
                                       nullptr,
                                       logger,
                                       frontendAuthenticator,
                                       backendAuthenticator))
{
}

/// Destructor
Proxy::~Proxy() = default;

/// Initialize the proxy
void Proxy::initialize(const ProxyOptions &parameters)
{
    if (!parameters.haveName())
    {
        throw std::invalid_argument("Proxy name not set");
    }
    auto proxyOptions = parameters.getProxyOptions();
    if (!proxyOptions.haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    if (!proxyOptions.haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    stop();
    // Set the proxy options
    pImpl->mOptions = parameters;
    pImpl->mProxyOptions = proxyOptions;
    pImpl->mProxy->initialize(pImpl->mProxyOptions);
    // Figure out the connection details
    pImpl->mConnectionDetails.setName(pImpl->mOptions.getName());
    auto proxyDetails = pImpl->mProxy->getSocketDetails();
    auto frontendDetails = proxyDetails.getXSubscriberFrontend();
    auto backendDetails  = proxyDetails.getXPublisherBackend();
    auto frontendPrivileges = UAuth::UserPrivileges::ReadOnly;
    auto backendPrivileges  = UAuth::UserPrivileges::ReadOnly;
    if (pImpl->mSymmetricAuthentication)
    {
        frontendPrivileges
            = pImpl->mAuthenticatorService->getMinimumUserPrivileges();
        backendPrivileges = frontendPrivileges;
    }
    else
    {
        frontendPrivileges
            = pImpl->mFrontendAuthenticatorService->getMinimumUserPrivileges();
        backendPrivileges 
            = pImpl->mBackendAuthenticatorService->getMinimumUserPrivileges();
    }
    frontendDetails.setMinimumUserPrivileges(frontendPrivileges);
    backendDetails.setMinimumUserPrivileges(backendPrivileges);
    proxyDetails.setSocketPair(std::pair(frontendDetails, backendDetails));

    pImpl->mConnectionDetails.setSocketDetails(proxyDetails);
    pImpl->mConnectionDetails.setConnectionType(UCI::ConnectionType::Broadcast);
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
