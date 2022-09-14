#include <string>
#include <mutex>
#include <zmq.hpp>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/services/command/remoteProxy.hpp"
#include "umps/services/command/remoteProxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/service.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"

#define SOCKET_MONITOR_DEALER_ADDRESS "inproc://monitor.dealer"

using namespace UMPS::Services::Command;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

namespace
{
class Monitor : public zmq::monitor_t
{
public:
    explicit Monitor(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger)
    {
    }
    void on_event_accepted(const zmq_event_t &, const char *addr) final
    {
        mLogger->debug("Monitor: Event opened: " + std::string{addr});
    }
    void on_event_closed(const zmq_event_t &, const char *addr) final
    {
        mLogger->debug("Monitor: Event closed: " + std::string{addr});
    }
    void on_event_disconnected(const zmq_event_t &, const char *addr) final
    {
        mLogger->debug("Monitor: Event disconnected: " + std::string{addr});
    }
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
};
}

class RemoteProxy::RemoteProxyImpl
{
public:
    /// C'tor - symmetric authentication
    RemoteProxyImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
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
        // Make the logger
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
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
        // Now make the sockets
        auto contextPtr
            = reinterpret_cast<zmq::context_t *> (mContext->getContext());
        mFrontend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                     zmq::socket_type::router);
        mFrontend->set(zmq::sockopt::router_mandatory, 1);
        mBackend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::dealer);
        // Create the authenticator service
        mAuthenticatorService = std::make_unique<UAuth::Service>
                (mContext, mLogger, mAuthenticator);
        // I only care about connections on the dealer
        mDealerMonitor = std::make_unique<Monitor> (mLogger);
    }
    // C'tor for asymmetric authentication
    RemoteProxyImpl(const std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
                    const std::shared_ptr<UMPS::Messaging::Context> &backendContext,
                    const std::shared_ptr<UMPS::Logging::ILog> &logger,
                    const std::shared_ptr<UAuth::IAuthenticator> &frontendAuthenticator,
                    const std::shared_ptr<UAuth::IAuthenticator> &backendAuthenticator) {
        // Handle context
        if (frontendContext == nullptr)
        {
            mFrontendContext = std::make_shared<UMPS::Messaging::Context>(1);
        }
        else
        {
            mFrontendContext = frontendContext;
        }
        if (backendContext == nullptr)
        {
            mBackendContext = std::make_shared<UMPS::Messaging::Context>(1);
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
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
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
        // Now make the sockets
        auto contextPtr
            = reinterpret_cast<zmq::context_t *> (mFrontendContext->getContext());
        mFrontend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                     zmq::socket_type::router);
        mFrontend->set(zmq::sockopt::router_mandatory, 1);
        contextPtr
            = reinterpret_cast<zmq::context_t *> (mBackendContext->getContext());
        mBackend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::dealer);
        // Make the authenticators
        mFrontendAuthenticatorService = std::make_unique<UAuth::Service>
                (mFrontendContext,
                 mLogger,
                 mFrontendAuthenticator);
        mBackendAuthenticatorService = std::make_unique<UAuth::Service>
                (mBackendContext,
                 mLogger,
                 mBackendAuthenticator);
        // I only care about connections on the dealer
        mDealerMonitor = std::make_unique<Monitor> (mLogger);
    }
    /// Starts the monitor
    void startMonitor()
    {
         mDealerMonitor->monitor(*mBackend,
                                 SOCKET_MONITOR_DEALER_ADDRESS,
                                 ZMQ_EVENT_ACCEPTED |
                                 ZMQ_EVENT_CLOSED |
                                 ZMQ_EVENT_DISCONNECTED);

    }
    /// Note whether the proxy was started / stopped
    void setStarted(const bool running)
    {
       std::scoped_lock lock(mMutex);
       mRunning = running;
    }
    /// True indicates the proxy is running or not
    bool isRunning() const
    {
       std::scoped_lock lock(mMutex);
       return mRunning;
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mFrontendContext{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mBackendContext{nullptr};
    std::unique_ptr<zmq::socket_t> mFrontend{nullptr};
    std::unique_ptr<zmq::socket_t> mBackend{nullptr};
    std::unique_ptr<Monitor> mDealerMonitor{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    std::unique_ptr<UAuth::Service> mFrontendAuthenticatorService{nullptr};
    std::unique_ptr<UAuth::Service> mBackendAuthenticatorService{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mFrontendAuthenticator{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mBackendAuthenticator{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    RemoteProxyOptions mOptions;
    UCI::SocketDetails::Proxy mSocketDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    bool mRunning{false};
    bool mInitialized{false};
    bool mSymmetricAuthentication{true};
};

/// C'tor
RemoteProxy::RemoteProxy() :
    pImpl(std::make_unique<RemoteProxyImpl> (nullptr, nullptr, nullptr))
{
}

RemoteProxy::RemoteProxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteProxyImpl> (nullptr, logger, nullptr))
{
}

RemoteProxy::RemoteProxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
                         std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<RemoteProxyImpl> (nullptr, logger, authenticator))
{
}

RemoteProxy::RemoteProxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
                         std::shared_ptr<UAuth::IAuthenticator> &frontendAuthenticator,
                         std::shared_ptr<UAuth::IAuthenticator> &backendAuthenticator) :
    pImpl(std::make_unique<RemoteProxyImpl> (nullptr,
                                             nullptr,
                                             logger,
                                             frontendAuthenticator,
                                             backendAuthenticator))
{
}

/// Destructor
RemoteProxy::~RemoteProxy() = default;

/// Initialize the proxy
void RemoteProxy::initialize(const RemoteProxyOptions &options)
{
    if (!options.haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    if (!options.haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    // Copy options
    pImpl->mOptions = options;
    pImpl->mInitialized = false;
    // Connect
    pImpl->mInitialized = true;
}

/// Initilalized?
bool RemoteProxy::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/*
/// Start the proxy
void RemoteProxy::start()
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    stop(); // Ensure proxy is stopped before starting
    // Poll setup
    constexpr size_t nPollItems = 2;
    zmq::pollitem_t items[] = 
    {
        {pImpl->mFrontend->handle(), 0, ZMQ_POLLIN, 0}, 
        {pImpl->mBackend->handle(),  0, ZMQ_POLLIN, 0}
    };
    pImpl->setStarted(true);
    pImpl->setStarted(true);
    while (isRunning())
    {   
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS); 
        // This is a request message -> route it to the right place
        if (items[0].revents & ZMQ_POLLIN)
        {
            try
            {
                zmq::multipart_t messagesReceived(*pImpl->mFrontend);
std::cout << "frontend received" << std::endl;
                for (const auto &item : messagesReceived)
                {
                    std::cout << item << std::endl;
                }
                messagesReceived.send(*pImpl->mBackend);
            }
            catch (const std::exception &e)
            {
                auto errorMsg = "Frontend to backend proxy error.  "
                              + std::string("ZeroMQ failed with:\n")
                              + std::string(e.what())
                              + " Error Code = " + std::to_string(e.num());
                pImpl->mLogger->error(errorMsg);
            }
        }
        // This is a response message -> send it back to the requestor
        if (items[1].revents & ZMQ_POLLIN)
        {
            try
            {
                zmq::multipart_t messagesReceived(*pImpl->mBackend);
std::cout << "backend received" << std::endl;
                for (const auto &item : messagesReceived)
                {
                    std::cout << item << std::endl;
                }
                messagesReceived.send(*pImpl->mFrontend);
            }
            catch (const zmq::error_t &e) //std::exception &e) 
            {
                auto errorMsg = "Backend to frontend proxy error.  "
                              + std::string("ZeroMQ failed with:\n")
                              + std::string(e.what())
                              + " Error Code = " + std::to_string(e.num());
                pImpl->mLogger->error(errorMsg); 
            }
        }
    } // Loop
}

void RemoteProxy::stop()
{

}
*/
