#include <iostream>
#include <array>
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
#include "umps/proxyServices/command/proxy.hpp"
#include "umps/proxyServices/command/proxyOptions.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/proxyServices/command/availableModulesRequest.hpp"
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/proxyServices/command/pingRequest.hpp"
#include "umps/proxyServices/command/pingResponse.hpp"
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

/*
std::vector<std::chrono::milliseconds> pingInterval
{
    10000,  // 10s
    30000,  // 30s
    600000, // 60s
};
*/

class Module
{
public:
    Module() = default;
    Module(const ModuleDetails &details, const std::string &address)
    {
        if (!details.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        if (address.empty())
        {
            throw std::invalid_argument("Address is empty");
        }
        mDetails = details;
        mAddress = address;
        // Just registered so there's a semblance of life
        updateLastResponseToNow();
    }
    /// @brief Updates the timing.  
    /// @note The const is just to make the std::set happy.  This won't
    ///       change the order.
    void updateLastResponseToNow() const 
    {
        auto now = std::chrono::high_resolution_clock::now();
        mLastResponse
            = std::chrono::duration_cast<std::chrono::milliseconds> (
                 now.time_since_epoch());
    }
    // Module details.
    ModuleDetails mDetails;
    // Address of module.
    std::string mAddress;
    // Last time module responded.
    mutable std::chrono::milliseconds mLastResponse{0};
};

struct CompareModules
{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return lhs < rhs;
    }
    bool operator()(const ::Module &lhs, const ::Module &rhs) const
    {
        std::string lString;
        if (lhs.mDetails.haveName()){lString = lhs.mDetails.getName();}
        std::string rString;
        if (rhs.mDetails.haveName()){rString = rhs.mDetails.getName();}
        return lString < rString;
    }
    bool operator()(const ::Module &lhs, const std::string &rString) const
    {
        std::string lString;
        if (lhs.mDetails.haveName()){lString = lhs.mDetails.getName();}
        return lString < rString;
    }
    bool operator()(const std::string &lString, const ::Module &rhs) const
    {
        std::string rString;
        if (rhs.mDetails.haveName()){rString = rhs.mDetails.getName();}
        return lString < rString;
    }
};

class Modules
{
public:
    [[nodiscard]] size_t size() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mModules.size();
    }
    [[nodiscard]] bool empty() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mModules.empty();
    }
    /// @result The backend's address corresponding to this module.
    /// @note If the address is empty then it was not found.
    [[nodiscard]]
    std::string getAddress(const std::string &moduleName) const noexcept
    {
        std::scoped_lock lock(mMutex);
        for (const auto &m : mModules)
        {
            if (m.mDetails.haveName())
            {
                if (moduleName == m.mDetails.getName()){return m.mAddress;}
            }
        }
        return "";
    }
    [[nodiscard]] bool contains(const std::string &moduleName) const
    {
        std::scoped_lock lock(mMutex);
        for (const auto &m : mModules)
        {
            if (m.mDetails.haveName())
            {
                if (moduleName == m.mDetails.getName()){return true;}
            }
            else
            {
                if (moduleName.empty()){return true;}
            }
        }
        return false;
    }
    [[nodiscard]] bool contains(const ModuleDetails &details) const
    {
        return contains(details.getName());
    }
    [[nodiscard]] bool contains(const ::Module &module) const
    {
        std::string moduleName;
        return mModules.contains(module);
    }
    void insert(Module &&module)
    {
        if (!module.mDetails.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        std::scoped_lock lock(mMutex);
        mModules.insert(std::move(module));
    }
    void insert(const Module &module)
    {
        if (!module.mDetails.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        std::scoped_lock lock(mMutex);
        mModules.insert(module);
    }
    void erase(const Module &module)
    {
        if (!module.mDetails.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        if (!contains(module)){return;} // Nothing to do, doesn't exist
        std::scoped_lock lock(mMutex);
        mModules.erase(module);
    }
    void update(const Module &module)
    {
        if (!module.mDetails.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        if (!contains(module))
        {
            insert(module);
        }
        else
        {
            erase(module);
            std::scoped_lock lock(mMutex);
            mModules.insert(module);
        }
    }
    void updateLastResponseToNow(const ::Module &module)
    {
        if (!module.mDetails.haveName()){return;}
        auto moduleName = module.mDetails.getName();
        std::scoped_lock lock(mMutex);
        auto m = mModules.find(module);
        if (m != mModules.end())
        {
            m->updateLastResponseToNow();
        }
    }
    [[nodiscard]] std::vector<ModuleDetails> toVector() const noexcept
    {
        std::vector<ModuleDetails> result;
        std::scoped_lock lock(mMutex);
        result.reserve(mModules.size());
        for (const auto &m : mModules)
        {
            result.push_back(m.mDetails);
        }
        return result;
    }
    mutable std::mutex mMutex;
    std::set<::Module, CompareModules> mModules;
};

}

class Proxy::ProxyImpl
{
public:
    /// @brief C'tor - symmetric authentication
    ProxyImpl(
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
    }
    /// @brief C'tor for asymmetric authentication
    ProxyImpl(
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
    }
    /// @brief Destructor
    ~ProxyImpl()
    {
        stop();
    }
    /// @brief Bind the frontend
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
    /// @brief Bind the backend.
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
    /// @brief Disconnect frontend.
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
    /// @brief Disconnect backend.
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
    /// @brief This thread periodically loops through the connected modules
    ///        and see who appears to be alive.
    void runModulePinger()
    {
        while (isRunning())
        {
            
            std::this_thread::sleep_for(std::chrono::milliseconds {100});
        }
    }
    /// @brief This is the main function that is the connects the clients
    ///        connected to the frontend and modules connected to the backend.
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
                    clientAddress = messagesReceived.at(0).to_string();
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

                        availableModulesResponse.setModules(
                            mModules.toVector());
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
                        if (messagesReceived.size() != 5)
                        {
                            throw std::runtime_error(
                            "Expecting request message of length 5.  Received: "
                          + std::to_string(messagesReceived.size()));
                        }
                        // Which module do they want to talk to?
                        mLogger->debug("Propagating message to backend...");
                        auto moduleName = messagesReceived.at(3).to_string();
                        // Router-router combinations are tricky.  We need to
                        // appropriately handle the routing by hand.  Details
                        // are in https://zguide.zeromq.org/docs/chapter3/
                        auto workerAddress = mModules.getAddress(moduleName);
                        if (!workerAddress.empty())
                        {
                            zmq::multipart_t moduleRequest;
                            moduleRequest.addstr(workerAddress);
                            // Ignore this so result shows up on backend
                            // as expected:
                            // Frame 1: Client
                            // Frame 2: Empty
                            // Frame 3: Message
                            //moduleRequest.addstr("");
                            moduleRequest.addstr(clientAddress);
                            moduleRequest.addstr("");
                            moduleRequest.addstr(messageType);
                            moduleRequest.push_back(
                                std::move(messagesReceived.at(4)));
                            moduleRequest.send(*mBackend);
                        }
                        else
                        {
                            // Handle invalid request
                            failureMessage.setDetails("Unknown module: "
                                                    + moduleName);
                            lSendError = true;
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
                    failureMessage.setDetails("ZeroMQ proxy error");
                    lSendError = true;
                }
                catch (const std::exception &e) 
                {
                    auto errorMsg = "Frontend to backend proxy std error:  "
                                  + std::string(e.what());
                    mLogger->error(errorMsg);
                    failureMessage.setDetails("Internal proxy error");
                    lSendError = true;
                }
                // Send an error message to the client if possible
                if (lSendError && !clientAddress.empty())
                {
                    zmq::multipart_t errorMessage;
                    errorMessage.addstr(clientAddress);
                    errorMessage.addstr("");
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
                    if (!messagesReceived.empty())
                    {
                        auto messageType = messagesReceived.at(1).to_string();
                        // This is a registration request
                        if (messageType == registrationRequest.getMessageType())
                        {
                            returnToClient = false; // Going back to backend
                            // Initialize the response
                            RegistrationResponse registrationResponse;
                            registrationResponse.setReturnCode(
                                RegistrationReturnCode::Success);
                            // Deserialize the request
                            auto workerAddress
                                = messagesReceived.at(0).to_string();
                            registrationRequest.fromMessage(
                                messagesReceived.at(2).to_string());
                            auto moduleDetails
                                = registrationRequest.getModuleDetails();
                            // Attempt to register the module
                            if (registrationRequest.getRegistrationType() ==
                                RegistrationType::Register)
                            {
                                if (!mModules.contains(moduleDetails))
                                {
                                    mModules.insert(
                                       ::Module{moduleDetails, workerAddress});
                                }
                                else
                                {
                                    registrationResponse.setReturnCode(
                                       RegistrationReturnCode::Exists);
                                }
                            }
                            else
                            {
                                mModules.erase(
                                   ::Module{moduleDetails, workerAddress});
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
                        } // End check on commuication with backend
                        // Business as usual - propagate result to client
                        if (returnToClient)
                        {
                            // Purge the the first address (that's the server's).
                            // Format is:
                            // 1. Client Address
                            // 2. Empty
                            // 3. Message [Header+Body; this is actually len 4]
                            messagesReceived.popstr();
                            messagesReceived.send(*mFrontend);
                        }
                    } // End check on non-empty message received
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
    /// @brief Starts the proxy.
    void start()
    {
        stop(); 
        setRunning(true);
        mProxyThread = std::thread(&ProxyImpl::runPoller, this);
        mModuleStatusThread = std::thread(&ProxyImpl::runModulePinger, this);
    }
    /// @brief Stops the proxy.
    void stop()
    {
        setRunning(false);
        if (mProxyThread.joinable()){mProxyThread.join();}
        if (mModuleStatusThread.joinable()){mModuleStatusThread.join();}
    }
    /// @brief Update connection details.
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
    /// @brief Note whether the proxy was started / stopped.
    void setRunning(const bool running)
    {
        std::scoped_lock lock(mMutex);
        mRunning = running;
    }
    /// @brief True indicates the proxy is running or not.
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
    // Intervals to check
    // First:  Sometimes things are missed.
    // Second: Warning; something appears wrong.
    // Third:  Dead; evict this module.
    // 1 + 4 + 5 = 10 seconds for module to respond. 
    std::array<std::chrono::seconds, 3>
        mModulePollIntervals{std::chrono::seconds{1},
                             std::chrono::seconds{4},
                             std::chrono::seconds{5}};
    // The registered modules
    ::Modules mModules;
    ProxyOptions mOptions;
    UCI::Details mConnectionDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mMonitorAddress;
    const std::string mProxyName{"RemoteCommandProxy"};
    std::thread mProxyThread;
    std::thread mModuleStatusThread;
    std::chrono::milliseconds mPollTimeOut{10};
    bool mHaveBackend{false};
    bool mHaveFrontend{false};
    bool mRunning{false};
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
void Proxy::initialize(const ProxyOptions &options)
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
    pImpl->disconnectFrontend();
    pImpl->disconnectBackend();
    // Connect
    pImpl->bindBackend();
    pImpl->bindFrontend();
    // Resolve the socket details
    pImpl->updateConnectionDetails();
    // Ready to rock
    pImpl->mInitialized = true;
    pImpl->mLogger->debug("Remote request proxy initialized!");
}

/// Initilalized?
bool Proxy::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Name
std::string Proxy::getName() const
{
    return pImpl->mProxyName;
}

/// Connection details
UCI::Details Proxy::getConnectionDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("ProxyService " + getName()
                               + " not initialized");
    }
    return pImpl->mConnectionDetails;
}

// Stops the proxy
void Proxy::stop()
{
    pImpl->setRunning(false);
}

/// Is the proxy running?
bool Proxy::isRunning() const noexcept
{
    return pImpl->isRunning();
}

/// Start the proxy
void Proxy::start()
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    stop(); // Ensure proxy is stopped before starting
    pImpl->start();
}
