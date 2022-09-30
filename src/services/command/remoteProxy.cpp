#include <iostream>
#include <string>
#include <set>
#include <mutex>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/services/command/remoteProxy.hpp"
#include "umps/services/command/remoteProxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/service.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "private/messaging/ipcDirectory.hpp"

//#define SOCKET_MONITOR_DEALER_ADDRESS "inproc://monitor.dealer"

using namespace UMPS::Services::Command;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

namespace
{
class Monitor : public zmq::monitor_t
{
public:
    /// Remove default c'tor
    Monitor() = delete;
    /// Constructor
    Monitor(std::shared_ptr<zmq::socket_t> &backend,
            std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mBackend(backend),
        mLogger(logger)
    {
        createInprocAddress();
    }
    /// Destructor
    ~Monitor() override
    {
        stop();
    }
    /// Stop the monitor
    void stop()
    {
        setRunning(false);
        if (mMonitorThread.joinable()){mMonitorThread.join();}
    }
    /// Start the monitor
    void start()
    {
        stop();
        mLogger->debug("Starting thread to monitor: " + getMonitorAddress());
        setRunning(true);
        mMonitorThread = std::thread([this]()
            {
                const std::chrono::microseconds timeOut{100};
                init(*this->mBackend, this->mMonitorAddress,
                     ZMQ_EVENT_ACCEPTED |
                     ZMQ_EVENT_CLOSED |
                     ZMQ_EVENT_DISCONNECTED);
                while (this->keepRunning())
                {
                    check_event(timeOut.count());
                }
                mLogger->debug("Monitor thread exited loop");
            });
    } 
    [[nodiscard]] bool keepRunning() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mKeepRunning;
    }
    void setRunning(const bool run)
    {
        std::scoped_lock lock(mMutex);
        mKeepRunning = run;
    }
    void on_monitor_started() override
    {
        mLogger->debug("Monitor thread started");
    }
    void on_event_accepted(const zmq_event_t &event, const char *addr) final
    {
        std::cout << event.event << " " << event.value << " " << std::hex << event.value << std::endl;
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
    /// @result The inproc address to talk to the background thread
    [[nodiscard]] std::string getMonitorAddress() const noexcept
    {
        return mMonitorAddress;
    }
    /// Create the inproc addresses
    void createInprocAddress()
    {
        // This is very unlikely to generate a collision.
        // Effectively the OS would have to overwrite this class's
        // location in memory.
        std::ostringstream address;
        address << static_cast<void const *> (this);
        auto nowMuSec
            = std::chrono::duration_cast<std::chrono::microseconds>
              (std::chrono::system_clock::now().time_since_epoch()).count();
        mMonitorAddress = "inproc://"
                        + std::to_string(nowMuSec)
                        + "_monitor_dealer";
    }
///private:
    mutable std::mutex mMutex; 
    std::set<std::string> mAddresses;
    std::shared_ptr<zmq::socket_t> mBackend{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::string mMonitorAddress;
    std::thread mMonitorThread;
    bool mKeepRunning{true};
};
}

class RemoteProxy::RemoteProxyImpl
{
public:
    /// C'tor - symmetric authentication
    RemoteProxyImpl(
        const std::shared_ptr<UMPS::Messaging::Context> &context,
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
        mBackend = std::make_shared<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::dealer);
        // Create the authenticator service
        mAuthenticatorService
            = std::make_unique<UAuth::Service>
              (mContext, mLogger, mAuthenticator);
        // I only care about connections on the dealer
        mDealerMonitor = std::make_unique<Monitor> (mBackend, mLogger);
    }
    /// C'tor for asymmetric authentication
    RemoteProxyImpl(
        const std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
        const std::shared_ptr<UMPS::Messaging::Context> &backendContext,
        const std::shared_ptr<UMPS::Logging::ILog> &logger,
        const std::shared_ptr<UAuth::IAuthenticator> &frontendAuthenticator,
        const std::shared_ptr<UAuth::IAuthenticator> &backendAuthenticator)
    {
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
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mFrontendContext->getContext());
        mFrontend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                     zmq::socket_type::router);
        mFrontend->set(zmq::sockopt::router_mandatory, 1);
        contextPtr = reinterpret_cast<zmq::context_t *>
                     (mBackendContext->getContext());
        mBackend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::dealer);
        // Make the authenticators
        mFrontendAuthenticatorService
            = std::make_unique<UAuth::Service>
              (mFrontendContext,
               mLogger,
               mFrontendAuthenticator);
        mBackendAuthenticatorService
            = std::make_unique<UAuth::Service>
              (mBackendContext,
               mLogger,
               mBackendAuthenticator);
        // I only care about connections on the dealer
        mDealerMonitor = std::make_unique<Monitor> (mBackend, mLogger);
    }
    /// Destructor
    ~RemoteProxyImpl()
    {
        stop();
        mDealerMonitor->stop();
    }
    /// Bind the frontend
    void bindFrontend()
    {
        mFrontendAddress = mOptions.getFrontendAddress();
        // Resolve a directory issue for IPC
        ::createIPCDirectoryFromConnectionString(mFrontendAddress, &*mLogger);
        try
        {
            mLogger->debug(
                "Remote request proxy attempting to bind to frontend: "
              + mFrontendAddress);
            mFrontend->set(zmq::sockopt::linger, 0);
            int hwm = mOptions.getFrontendHighWaterMark();
            if (hwm > 0)
            {
                mFrontend->set(zmq::sockopt::sndhwm, hwm);
                mFrontend->set(zmq::sockopt::rcvhwm, hwm);
            }
            mFrontend->bind(mFrontendAddress);
            mHaveFrontend = true;
        }
        catch (const std::exception &e)
        {
            auto errorMsg = "Remote request proxy failed to bind to frontend: "
                          + mFrontendAddress + ".\nZeroMQ failed with:\n"
                          + std::string(e.what());
            mLogger->error(errorMsg);
            throw std::runtime_error(errorMsg);
        }
        // Resolve the frontend address
        if (mHaveFrontend)
        {
            if (mFrontendAddress.find("tcp") != std::string::npos ||
                mFrontendAddress.find("ipc") != std::string::npos)
            {
                mFrontendAddress = mFrontend->get(zmq::sockopt::last_endpoint);
            }
        }
    }
    /// Bind the backend
    void bindBackend()
    {
        mBackendAddress = mOptions.getBackendAddress();
        // Resolve a directory issue for IPC
        ::createIPCDirectoryFromConnectionString(mBackendAddress, &*mLogger);
        try
        {
            mLogger->debug(
                "Remote request proxy attempting to bind to backend: "
              + mBackendAddress);
            mBackend->set(zmq::sockopt::linger, 0);
            int hwm = mOptions.getBackendHighWaterMark();
            if (hwm >= 0)
            {
                mBackend->set(zmq::sockopt::sndhwm, hwm);
                mBackend->set(zmq::sockopt::rcvhwm, hwm);
            }
            mBackend->bind(mBackendAddress);
            mHaveBackend = true;
        }
        catch (const std::exception &e)
        {
            auto errorMsg = "Remote request proxy failed to bind to backend: "
                          + mBackendAddress
                          + ".\nZeroMQ failed with:\n" + std::string(e.what());
            mLogger->error(errorMsg);
            throw std::runtime_error(errorMsg);
        }
        // Resolve the backend address
        if (mHaveBackend)
        {
            if (mBackendAddress.find("tcp") != std::string::npos ||
                mBackendAddress.find("ipc") != std::string::npos)
            {
                mBackendAddress = mBackend->get(zmq::sockopt::last_endpoint);
            }
        }
    }
    /// Disconnect frontend
    void disconnectFrontend()
    {
        if (mHaveFrontend)
        {
            ::removeIPCFile(mFrontendAddress, &*mLogger);
            mLogger->debug("Disconnecting from current frontend: "
                         + mFrontendAddress);
            mFrontend->disconnect(mFrontendAddress);
            mHaveFrontend = false;
        }
    }
    /// Disconnect backend
    void disconnectBackend()
    {
        if (mHaveBackend)
        {
            ::removeIPCFile(mBackendAddress, &*mLogger);
            mLogger->debug("Disconnecting from current backend: "
                         + mBackendAddress);
            mBackend->disconnect(mBackendAddress);
            mHaveBackend = false;
        }
    }
    /// Starts the monitor
    void startMonitor()
    {
        mLogger->debug("Remote request proxy starting backend monitor...");
        mDealerMonitor->start();
        mLogger->debug("Remote request proxy monitor started");
    }
    /// Stops the mointor
    void stopMonitor()
    {
        mDealerMonitor->stop();
    }
    /// Starts the poller
    void runPoller()
    {
        // Poll setup
        constexpr size_t nPollItems = 2;
        zmq::pollitem_t items[] =
        {
            {mFrontend->handle(), 0, ZMQ_POLLIN, 0},
            {mBackend->handle(),  0, ZMQ_POLLIN, 0}
        };
        // Run
        while (isRunning())
        {
            zmq::poll(&items[0], nPollItems, mPollTimeOut); 
            // This is a request message -> route it to the right place
            if (items[0].revents & ZMQ_POLLIN)
            {
//std::cout << "frontend received" << std::endl;
                try
                {
                    zmq::multipart_t messagesReceived(*mFrontend);
std::cout << "Received: " << messagesReceived << std::endl << std::endl;//messagesReceived.front().size() << std::endl;
                    messagesReceived.send(*mBackend);
                }
                catch (const zmq::error_t &e)
                {
                    auto errorMsg = "Frontend to backend proxy error.  "
                                  + std::string("ZeroMQ failed with:\n")
                                  + std::string(e.what())
                                  + " Error Code = " + std::to_string(e.num());
                    mLogger->error(errorMsg);
                }
                catch (std::exception &e) 
                {
                    auto errorMsg = "Frontend to backend proxy std error:  "
                                  + std::string(e.what());
                    mLogger->error(errorMsg);
                }
            }
            // This is a response message -> send it back to the requestor
            if (items[1].revents & ZMQ_POLLIN)
            {
//std::cout << "backend received" << std::endl;
                try
                {
                    zmq::multipart_t messagesReceived(*mBackend);

//const auto identity = reinterpret_cast<const char *> (messagesReceived.front().data());
//std::string identity{reinterpret_cast<const char *> ( messagesReceived.front().data() ), messagesReceived.front().size()};
//std::cout << "Return: " << messagesReceived.front().size() << " (" << identity << ")" << std::endl;
 //messagesReceived.front().str() << std::endl;
std::cout <<"Returned: " << messagesReceived << std::endl << std::endl;
                    messagesReceived.send(*mFrontend);
                }
                catch (const zmq::error_t &e) //std::exception &e) 
                {
                    auto errorMsg = "Backend to frontend proxy error.  "
                                  + std::string("ZeroMQ failed with:\n")
                                  + std::string(e.what())
                                  + " Error Code = " + std::to_string(e.num());
                    mLogger->error(errorMsg); 
                }
                catch (std::exception &e)
                {
                    auto errorMsg = "Backend to frontend proxy std error:  "
                                  + std::string(e.what());
                    mLogger->error(errorMsg);
                }
            } 
        }
    }
    /// Starts the proxy
    void start()
    {
        stop(); 
        setRunning(true);
        mProxyThread = std::thread(&RemoteProxyImpl::runPoller, this);
    }
    /// Stops the proxy
    void stop()
    {
        setRunning(false);
        if (mProxyThread.joinable()){mProxyThread.join();}
    }
    /// Update connection details
    void updateConnectionDetails()
    {
        UCI::SocketDetails::Proxy socketDetails;
        auto securityLevel = mOptions.getZAPOptions().getSecurityLevel();
        UCI::SocketDetails::Router router;
        router.setAddress(mFrontendAddress);
        router.setSecurityLevel(securityLevel);
        router.setConnectOrBind(UCI::ConnectOrBind::Connect);

        UCI::SocketDetails::Dealer dealer;
        dealer.setAddress(mBackendAddress);
        dealer.setSecurityLevel(securityLevel);
        dealer.setConnectOrBind(UCI::ConnectOrBind::Connect);

        if (mSymmetricAuthentication)
        {
            auto privileges = mAuthenticator->getMinimumUserPrivileges();
            router.setMinimumUserPrivileges(privileges);
            dealer.setMinimumUserPrivileges(privileges); 
        }
        else
        {
            router.setMinimumUserPrivileges(
                mFrontendAuthenticator->getMinimumUserPrivileges());
            dealer.setMinimumUserPrivileges(
                mBackendAuthenticator->getMinimumUserPrivileges());
        }
        socketDetails.setSocketPair(std::pair{router, dealer});
        // Set the connection details
        mConnectionDetails.setName(mProxyName);
        mConnectionDetails.setSocketDetails(socketDetails);
        mConnectionDetails.setConnectionType(UCI::ConnectionType::Service);
        mConnectionDetails.setSecurityLevel(securityLevel);
    }
    /// Note whether the proxy was started / stopped
    void setRunning(const bool running)
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
    // Context that controls external communication for symmetric authentication
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    // The router's context for assymetric authentication
    std::shared_ptr<UMPS::Messaging::Context> mFrontendContext{nullptr};
    // The dealer's context for assymetric authentication
    std::shared_ptr<UMPS::Messaging::Context> mBackendContext{nullptr};
    // The frontend router socket
    std::unique_ptr<zmq::socket_t> mFrontend{nullptr};
    // The backend dealer socket
    std::shared_ptr<zmq::socket_t> mBackend{nullptr};
    // Monitor's the backend cocnnections
    std::unique_ptr<Monitor> mDealerMonitor{nullptr};
    // Authentication service for symmetric authentication
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    // Authentication service for router for assymetric authentication
    std::unique_ptr<UAuth::Service> mFrontendAuthenticatorService{nullptr};
    // Authentication service for dealer for assymetric authentication
    std::unique_ptr<UAuth::Service> mBackendAuthenticatorService{nullptr};
    // The authenticator used by the authenticator service
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    // The authenticator used by the frontend authentciator service
    std::shared_ptr<UAuth::IAuthenticator> mFrontendAuthenticator{nullptr};
    // The authenticator used by the backend authenticator service
    std::shared_ptr<UAuth::IAuthenticator> mBackendAuthenticator{nullptr};
    // Logger
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    RemoteProxyOptions mOptions;
    UCI::Details mConnectionDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mMonitorAddress;
    const std::string mProxyName{"RemoteCommandProxy"};
    std::thread mProxyThread;
    std::chrono::milliseconds mPollTimeOut{10};
    bool mHaveBackend{false};
    bool mHaveFrontend{false};
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
    // Disconnect from old connections
    pImpl->stopMonitor();
    pImpl->disconnectFrontend();
    pImpl->disconnectBackend();
    // Connect
    pImpl->bindBackend();
    pImpl->bindFrontend();
    // Resolve the socket details
    pImpl->updateConnectionDetails();
    // Ready to rock
    pImpl->startMonitor();
    pImpl->mInitialized = true;
    pImpl->mLogger->debug("Remote request proxy initialized!");
}

/// Initilalized?
bool RemoteProxy::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Name
std::string RemoteProxy::getName() const
{
    return pImpl->mProxyName;
}

/// Connection details
UCI::Details RemoteProxy::getConnectionDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("ProxyService " + getName()
                               + " not initialized");
    }
    return pImpl->mConnectionDetails;
}

// Stops the proxy
void RemoteProxy::stop()
{
    pImpl->setRunning(false);
}

/// Is the proxy running?
bool RemoteProxy::isRunning() const noexcept
{
    return pImpl->isRunning();
}

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
    //pImpl->setRunning(true);
    //pImpl->mLogger->debug("Remote proxy thread beginning polling loop...");
    pImpl->start();    
/*
    while (isRunning())
    {
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOut); 
        // This is a request message -> route it to the right place
        if (items[0].revents & ZMQ_POLLIN)
        {
std::cout << "frontend received" << std::endl;
*/
/*
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
            catch (const zmq::error_t &e)
            {
                auto errorMsg = "Frontend to backend proxy error.  "
                              + std::string("ZeroMQ failed with:\n")
                              + std::string(e.what())
                              + " Error Code = " + std::to_string(e.num());
                pImpl->mLogger->error(errorMsg);
            }
            catch (std::exception &e) 
            {
                auto errorMsg = "Frontend to backend proxy std error:  "
                              + std::string(e.what());
                pImpl->mLogger->error(errorMsg);
            }
*/
/*
        }
        // This is a response message -> send it back to the requestor
        if (items[1].revents & ZMQ_POLLIN)
        {
std::cout << "backend received" << std::endl;
*/
/*
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
            catch (std::exception &e)
            {
                auto errorMsg = "Backend to frontend proxy std error:  "
                              + std::string(e.what());
                pImpl->mLogger->error(errorMsg);
            }
*/
/*
        }
    } // Loop
    pImpl->mLogger->debug("Remote request thread exited polling loop.");
*/
}
