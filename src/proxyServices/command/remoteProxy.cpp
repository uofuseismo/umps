#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/proxyServices/command/remoteProxy.hpp"
#include "umps/proxyServices/command/remoteProxyOptions.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/proxyServices/command/availableModulesRequest.hpp"
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/proxyServices/command/registrationRequest.hpp"
#include "umps/proxyServices/command/registrationResponse.hpp"
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
#include "umps/messageFormats/failure.hpp"
#include "umps/logging/stdout.hpp"
#include "private/messaging/ipcDirectory.hpp"

//#define SOCKET_MONITOR_DEALER_ADDRESS "inproc://monitor.dealer"

using namespace UMPS::ProxyServices::Command;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

namespace
{

struct Comparator
{
    bool operator()(const ModuleDetails &lhs, const ModuleDetails &rhs) const
    {
        if (lhs.haveName() == rhs.haveName())
        {
            if (lhs.haveName())
            {
                return (lhs.getName() < rhs.getName());
            }
        }
        return false;
    }
};

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
        //std::cout << event.event << " " << event.value << " " << std::hex << event.value << std::endl;
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

struct Module
{
    std::string mRoutingIdentifier;
    ModuleDetails moduleDetails;
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
                                                    zmq::socket_type::router);
        // Create the authenticator service
        mAuthenticatorService
            = std::make_unique<UAuth::Service>
              (mContext, mLogger, mAuthenticator);
        // I only care about connections on the dealer
        mBackendMonitor = std::make_unique<Monitor> (mBackend, mLogger);
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
        mBackendMonitor = std::make_unique<Monitor> (mBackend, mLogger);
    }
    /// Destructor
    ~RemoteProxyImpl()
    {
        stop();
        mBackendMonitor->stop();
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
        mBackendMonitor->start();
        mLogger->debug("Remote request proxy monitor started");
    }
    /// Stops the mointor
    void stopMonitor()
    {
        mBackendMonitor->stop();
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
        UMPS::MessageFormats::Failure failureMessage;
        AvailableModulesRequest availableModulesRequest;
        RegistrationRequest registrationRequest;
        while (isRunning())
        {
            zmq::poll(&items[0], nPollItems, mPollTimeOut); 
            //----------------------------------------------------------------//
            //                     Message From Frontend                      //
            //----------------------------------------------------------------//
            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t messagesReceived;
                std::string clientAddress;
                bool lSendError{false};
                try
                {
                    messagesReceived.recv(*mFrontend);
                    // Get client address and message type
                    auto clientAddress = messagesReceived.at(0).to_string();
                    auto messageType = messagesReceived.at(2).to_string();
                    // Handle an available request message
                    if (messageType == availableModulesRequest.getMessageType())
                    {
                        mLogger->debug("Handling available modules request...");
                        const auto payload
                            = static_cast<const char *>
                              (messagesReceived.at(3).data());
                        availableModulesRequest.fromMessage(
                            payload, messagesReceived.at(3).size());
                        AvailableModulesResponse availableModulesResponse;

                        std::vector<ModuleDetails> moduleDetails;
                        moduleDetails.reserve(mModules.size());
                        for (const auto &module : mModules)
                        {
                            moduleDetails.emplace_back(module.first);
                        }
                        availableModulesResponse.setModules(
                            std::move(moduleDetails));
                        availableModulesResponse.setIdentifier(
                            availableModulesRequest.getIdentifier());

                        zmq::multipart_t response;
                        response.addstr(clientAddress);
                        response.addstr("");
                        response.addstr(
                            availableModulesResponse.getMessageType());
                        response.addstr(availableModulesResponse.toMessage());
                        response.send(*mFrontend);
                    }
                    else
                    {
                        // Which module do they want to talk to?
                        mLogger->debug("Propagating message to backend...");
                        // Router-router combinations are tricky.  We need to
                        // appropriately handle the routing by hand.  Details
                        // are in https://zguide.zeromq.org/docs/chapter3/
                        ModuleDetails searchForModule;
                        searchForModule.setName("asdf"); 
                        auto idx = mModules.find(searchForModule);
                        if (idx != mModules.end())
                        {
                            auto workerAddress = idx->second; 
std::cout << messagesReceived << std::endl;
                            zmq::multipart_t moduleRequest;
                            moduleRequest.addstr(workerAddress);
                            moduleRequest.addstr(""); // Empty frame
                            moduleRequest.addstr(clientAddress);
                            moduleRequest.addstr(""); // Empty frame 
                            moduleRequest.addstr(messageType);
                            moduleRequest.push_back(
                                std::move(messagesReceived.at(3)));
std::cout << "Propagating: " << moduleRequest << std::endl << std::endl;
                            moduleRequest.send(*mBackend);
                        }
                        else
                        {
                            // Handle invalid request
                            UMPS::MessageFormats::Failure failureMessage;
                            failureMessage.setDetails(
                                "Unknown module: " + searchForModule.getName());
                            zmq::multipart_t failureResponse;
                            failureResponse.addstr(clientAddress);
                            failureResponse.addstr(
                                failureMessage.getMessageType());
                            failureResponse.addstr(failureMessage.toMessage());
                            failureResponse.send(*mFrontend);
                        }
                    }
                }
                catch (const zmq::error_t &e)
                {
                    auto errorMsg = "Frontend to backend proxy error.  "
                                  + std::string("ZeroMQ failed with:\n")
                                  + std::string(e.what())
                                  + " Error Code = " + std::to_string(e.num());
                    mLogger->error(errorMsg);
                    lSendError = true;
                }
                catch (const std::exception &e) 
                {
                    auto errorMsg = "Frontend to backend proxy std error:  "
                                  + std::string(e.what());
                    mLogger->error(errorMsg);
                    lSendError = true;
                }
                // Send an error message to the client if possible
                if (lSendError && !clientAddress.empty())
                {
                    zmq::multipart_t errorMessage;
                    errorMessage.addstr(clientAddress);
                    errorMessage.addstr(failureMessage.getMessageType());
                    errorMessage.addstr(failureMessage.toMessage());
                    try
                    {
                        errorMessage.send(*mFrontend); 
                    }
                    catch (const std::exception &e)
                    {
                        mLogger->error("Failed to send error message to: "
                                     + clientAddress 
                                     + ".  Failed with: " + e.what());
                    }
                }
            }
            //----------------------------------------------------------------//
            //                       Message From Backend                     //
            //----------------------------------------------------------------//
            if (items[1].revents & ZMQ_POLLIN)
            {
                try
                {
                    bool returnToClient{true};
                    zmq::multipart_t messagesReceived(*mBackend);
                    // Coming from reply socket
                    if (messagesReceived.size() > 1)
                    {
                        auto messageType = messagesReceived.at(1).to_string();
                        // This is a registration request
                        if (messageType == registrationRequest.getMessageType())
                        {
                            // Initialize the response
                            RegistrationResponse registrationResponse;
                            registrationResponse.setReturnCode(
                                RegistrationReturnCode::Success);
                            returnToClient = false; // Going back to backend
                            // Deserialize the request
                            auto workerAddress
                                = messagesReceived.at(0).to_string();
                            registrationRequest.fromMessage(
                                messagesReceived.at(2).to_string());
                            auto moduleDetails
                                = registrationRequest.getModuleDetails();
                            // Attempt to register the module
                            if (!mModules.contains(moduleDetails))
                            {
                                mModules.insert(
                                   std::pair {moduleDetails, workerAddress});
                            }
                            else
                            {
                                registrationResponse.setReturnCode(
                                   RegistrationReturnCode::Exists);
                            }
                            // Create a reply and send it
                            zmq::multipart_t registrationReply;
                            registrationReply.addstr(workerAddress);
                            //registrationReply.addstr(""); // Empty frame
                            registrationReply.addstr(
                                registrationResponse.getMessageType());
                            registrationReply.addstr(
                                registrationResponse.toMessage());
                            registrationReply.send(*mBackend);
                        }
                    }
                    // Business as usual - propagate result to client
                    if (returnToClient)
                    {
                        messagesReceived.send(*mFrontend);
                    }
                }
                catch (const zmq::error_t &e)
                {
                    auto errorMsg = "Backend to frontend proxy error.  "
                                  + std::string("ZeroMQ failed with:\n")
                                  + std::string(e.what())
                                  + " Error Code = " + std::to_string(e.num());
                    mLogger->error(errorMsg); 
                }
                catch (const std::exception &e)
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
    std::unique_ptr<Monitor> mBackendMonitor{nullptr};
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
    // The registered modules
    std::map<ModuleDetails, std::string, Comparator> mModules;
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
    pImpl->start();
}
