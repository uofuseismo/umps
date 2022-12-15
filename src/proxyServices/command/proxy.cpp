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
#include "umps/logging/standardOut.hpp"
#include "private/messaging/ipcDirectory.hpp"
#include "private/services/ping.hpp"
#include "private/services/terminate.hpp"
#include "private/threadSafeQueue.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

namespace
{

class Module
{
public:
    Module() = default;
    Module(const ModuleDetails &details,
           const std::vector<std::chrono::milliseconds> &pingIntervals) :
        mPingIntervals(pingIntervals)
    {
        if (!details.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        mDetails = details;
        mJustPinged.resize(pingIntervals.size(), false);
        // Just registered so there's a semblance of life
        updateLastResponseToNow();
    }
    /// @brief Updates the timing.  
    void updateLastResponseToNow()
    {
        auto now = std::chrono::high_resolution_clock::now();
        mLastResponse
            = std::chrono::duration_cast<std::chrono::milliseconds> (
                 now.time_since_epoch());
        std::fill(mJustPinged.begin(), mJustPinged.end(), false);
    }
    // Module details.
    ModuleDetails mDetails;
    // Last time module responded.
    std::chrono::milliseconds mLastResponse{0};
    // Last time I pinged the module
    std::chrono::milliseconds mLastPing{0};
    // The ping intervals
    std::vector<std::chrono::milliseconds> mPingIntervals;
    // True indicates the module was just pinged
    std::vector<bool> mJustPinged;
};

/// @brief Thread-safe map between addresses and modules.
///        Technically, both the worker address and module are unique but
///        it proves to be slightly simpler to make the worker address the
///        key in the map.
class ModulesMap
{
public:
    /// @brief Utility to verify that input worker address and module
    ///        details are valid.
    void checkAndThrow(const std::pair<std::string, ::Module> &item)
    {
        if (!item.second.mDetails.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        if (contains(item.first))
        {
            throw std::invalid_argument("Worker already exists");
        }
        if (contains(item.second))
        {
            throw std::invalid_argument("Module already exists");
        }
   }
    void insert(std::pair<std::string, ::Module> &&item)
    {
        checkAndThrow(item);
        std::scoped_lock lock(mMutex);
        mModules.insert(std::move(item));
    }
    void insert(const std::pair<std::string, ::Module> &item)
    {
        checkAndThrow(item);
        std::scoped_lock lock(mMutex);
        mModules.insert(item);
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
            if (m.second.mDetails.haveName())
            {
                if (moduleName == m.second.mDetails.getName()){return m.first;}
            }
        }
        return "";
    }
    /// @result True indicates the module is in the map.
    [[nodiscard]] bool contains(const ::Module &module) const noexcept
    {
         return contains(module.mDetails);
    }
    /// @result True indicates the module is in the map.
    [[nodiscard]] bool contains(const ModuleDetails &details) const noexcept
    {
        std::string moduleName{""};
        if (details.haveName())
        {
            moduleName = details.getName();
        }
        auto instance = details.getInstance();
        std::scoped_lock lock(mMutex);
        for (const auto &m : mModules)
        {
            auto instanceCompare = m.second.mDetails.getInstance();
            if (instance == instanceCompare)
            {
                std::string moduleNameCompare{""};
                if (m.second.mDetails.haveName())
                {
                    moduleNameCompare = m.second.mDetails.getName(); 
                }
                if (moduleName == moduleNameCompare)
                {
                    return true;
                }
            }
        }
        return false;
    }
    /// @brief Updates the last ping time for the module at this address.
    void updateLastResponseToNow(const std::string &address)
    {
        std::scoped_lock lock(mMutex);
        auto idx = mModules.find(address);
        if (idx != mModules.end()){idx->second.updateLastResponseToNow();}
    } 
    /// @result True indicates the worker address is in the map.
    [[nodiscard]] bool contains(const std::string &workerAddress) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mModules.contains(workerAddress);
    }
    /// @brief Removes a module corresponding to the ZMQ address.
    void erase(const std::string &workerAddress)
    {
        if (workerAddress.empty())
        {
            throw std::invalid_argument("Worker address is empty");
        }
        if (!contains(workerAddress)){return;} // Nothing to do, doesn't exist
        std::scoped_lock lock(mMutex);
        mModules.erase(workerAddress);
    }
    /// @result A list of modules.
    [[nodiscard]] std::vector<ModuleDetails> toVector() const noexcept
    {
        std::vector<ModuleDetails> result;
        std::scoped_lock lock(mMutex);
        result.reserve(mModules.size());
        for (const auto &m : mModules)
        {
            result.push_back(m.second.mDetails);
        }
        return result;
    }
    mutable std::mutex mMutex;
    std::map<std::string, ::Module> mModules;
};

}

///--------------------------------------------------------------------------///
///                                 Implementation                           ///
///--------------------------------------------------------------------------///
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
        // Now make the sockets
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mFrontendContext->getContext());
        mFrontend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                     zmq::socket_type::router);
        mFrontend->set(zmq::sockopt::router_mandatory, 1);
        contextPtr = reinterpret_cast<zmq::context_t *>
                     (mBackendContext->getContext());
        mBackend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::router);
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
        std::pair<std::string, PingResponse> pingResponses;
        std::queue<std::string> killQueue;
        while (isRunning())
        {
            // Process responses in my queue
            bool moreWork{true};
            while (moreWork)
            {
                auto workItem = mPingResponses.try_pop();
                if (workItem != nullptr)
                {
                    // Find this item
                    auto workerAddress = workItem->first;
                    // Update the ping request time
                    mModulesMap.updateLastResponseToNow(workerAddress);
                }
                else
                {
                    moreWork = false;
                }
            }
            // If I haven't heard from you in awhile then you get a ping
            ::PingRequest pingRequest;
            auto now = pingRequest.getTime();
            for (auto &m : mModulesMap.mModules)
            {
                auto dt = now - m.second.mLastResponse;
                // Expired - remove it from the module list
                if (dt > mModuleTimeOutInterval)
                {
                    ::TerminateRequest terminateRequest;
                    mTerminateRequests.push(std::pair{m.first,
                                                      terminateRequest});
                    killQueue.push(m.first);
                    mLogger->warn("No response from: "
                                + m.second.mDetails.getName()
                                + ".  Removing it from list.");
                }
                else // Time for a ping?
                {
                    // Hunt for first available ping time
                    for (size_t i = 0; i < m.second.mPingIntervals.size(); ++i)
                    {
                        if (dt > m.second.mPingIntervals[i] &&
                            !m.second.mJustPinged[i])
                        {
                            m.second.mJustPinged[i] = true;
                            mPingRequests.push(std::pair{m.first, pingRequest});
                            break;
                        }
                    }
                }
            }
            // Evict terminated modules
            while (!killQueue.empty())
            {
                mModulesMap.erase(killQueue.front());
                killQueue.pop();
            }
            // My life is hard.  Time for a nap.
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
                            mModulesMap.toVector());
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
                        auto workerAddress = mModulesMap.getAddress(moduleName);
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
                    auto errorMsg = "Frontend to backend proxy std error: "
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
                    __handleMessagesFromBackend();
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
                    auto errorMsg = "Backend to frontend proxy std error: "
                                  + std::string(e.what());
                    mLogger->error(errorMsg);
                }
            } // End check on backend poller
            //----------------------------------------------------------------//
            //                      Send Ping Requests                        //
            //----------------------------------------------------------------//
            bool sendPingRequests{true};
            while (sendPingRequests)
            {
                auto workItem = mPingRequests.try_pop();
                if (workItem != nullptr)
                {
                    __sendPingRequestToServer(workItem->first,
                                              workItem->second);
                }
                else
                {
                    sendPingRequests = false;
                }
            }
            //----------------------------------------------------------------//
            //                        Send Terminate Requests                 //
            //----------------------------------------------------------------//
            bool sendTerminateRequests{true};
            while (sendTerminateRequests)
            {
                auto workItem = mTerminateRequests.try_pop();
                if (workItem != nullptr)
                {
                    __sendTerminateRequestToServer(workItem->first,
                                                   workItem->second);
                }
                else
                {
                    sendTerminateRequests = false;
                }
            }
        } // while isRunning()
        // Give other threads a chance to quit
        std::this_thread::sleep_for(std::chrono::milliseconds {250});
        // Send terminate commands
        if (!mModulesMap.empty())
        {
            ::TerminateRequest privateTerminateRequest;
            mLogger->debug("Evicting modules...");
            for (const auto &m : mModulesMap.mModules)
            {
                __sendTerminateRequestToServer(m.first,
                                               privateTerminateRequest);
            }
            // Wait for other threads to process
            std::this_thread::sleep_for(std::chrono::milliseconds {250});
            bool waitForMoreResponses{true};
            while (waitForMoreResponses)
            {
                zmq::poll(&items[0], nPollItems, mPollTimeOut);
                if (items[1].revents & ZMQ_POLLIN)
                {
                    try
                    {
                        __handleMessagesFromBackend();
                    }
                    catch (const zmq::error_t &e)
                    {
                        auto errorMsg = "Backend cleanup error.  "
                                      + std::string("ZeroMQ failed with:\n")
                                      + std::string(e.what())
                                      + " Error Code = " + std::to_string(e.num());
                        mLogger->error(errorMsg);
                    }
                    catch (const std::exception &e)
                    {
                        auto errorMsg = "Backend cleanup std error: "
                                      + std::string(e.what());
                        mLogger->error(errorMsg);
                    }
                    waitForMoreResponses = true;
                }
                else
                {
                    waitForMoreResponses = false;
                }
            }
        }
        // Warn about maybe who didn't deregister
        for (const auto &m : mModulesMap.mModules)
        {
            mLogger->warn(m.second.mDetails.getName()
                        + " may still be running"); 
        }
    } // End function
    /// @brief Sends a ping message to the backend.
    void __sendPingRequestToServer(const std::string &address,
                                   const ::PingRequest &request)
    {
        zmq::multipart_t pingMessage;
        pingMessage.addstr(address);
        pingMessage.addstr(address); // Reply to me
        pingMessage.addstr("");
        pingMessage.addstr(request.getMessageType());
        pingMessage.addstr(request.toMessage());
        try
        {
            pingMessage.send(*mBackend);
        }
        catch (...)
        {
            mLogger->error("Failed to send ping to: " + address);
        }
    }
    /// @brief Sends a terminate message to the backend.
    void __sendTerminateRequestToServer(const std::string &address,
                                        const ::TerminateRequest &request)
    {
        zmq::multipart_t terminateMessage;
        terminateMessage.addstr(address);
        terminateMessage.addstr(address); // Reply to me
        terminateMessage.addstr("");
        terminateMessage.addstr(request.getMessageType());
        terminateMessage.addstr(request.toMessage());
        try
        {
            terminateMessage.send(*mBackend);
        }
        catch (...)
        {
            mLogger->error("Failed to send terminate to: " + address);
        }
    }
    /// @brief Private function to handle a ping response.
    void __handlePingResponse(const zmq::multipart_t &messagesReceived)
    {
        ::PingResponse pingResponse;
        // Purely for internal consumption
        auto workerAddress = messagesReceived.at(0).to_string();
        const auto payload = static_cast<const char *>
                             (messagesReceived.at(2).data());
        try
        {
            pingResponse.fromMessage(payload,
                                     messagesReceived.at(2).size());
        }
        catch (const std::exception &e)
        {
            mLogger->error("Failed to deserialize ping from " + workerAddress);
            return;
        }
        mPingResponses.push(std::pair{workerAddress, pingResponse});
    }
    /// @brief Private function to handle a terminate request.
    void __handleTerminateResponse(const zmq::multipart_t &messagesReceived)
    {
        // If not already deleted then do it now
        auto workerAddress = messagesReceived.at(0).to_string();
        mModulesMap.erase(workerAddress);
    }
    /// @brief Private function to handle a (de)registration response
    void __handleRegistrationRequest(const zmq::multipart_t &messagesReceived)
    {
        if (messagesReceived.size() != 3){return;} // Wrong size
        // Initialize the response
        RegistrationRequest registrationRequest;
        RegistrationResponse registrationResponse;
        registrationResponse.setReturnCode(RegistrationReturnCode::Success);
        // Deserialize the request
        auto workerAddress = messagesReceived.at(0).to_string();
        const auto payload = static_cast<const char *>
                             (messagesReceived.at(2).data());
        registrationRequest.fromMessage(payload, messagesReceived.at(2).size());
        auto moduleDetails = registrationRequest.getModuleDetails();
        // Attempt to register the module
        if (registrationRequest.getRegistrationType() ==
            RegistrationType::Register)
        {
            if (!mModulesMap.contains(moduleDetails))
            {
                mLogger->debug("Registering: "
                             + workerAddress);
                mModulesMap.insert(std::pair{workerAddress,
                                             ::Module(moduleDetails,
                                             mOptions.getPingIntervals())});
            } 
            else
            {
                registrationResponse.setReturnCode(
                    RegistrationReturnCode::Exists);
            }
        }
        else // De-register
        {
            // Whether it exists or not this is a success
            mLogger->debug("Deregistering: " + workerAddress);
            mModulesMap.erase(workerAddress);
        }
        // Create a reply and send it
        zmq::multipart_t registrationReply;
        registrationReply.addstr(workerAddress);
        registrationReply.addstr(registrationResponse.getMessageType());
        registrationReply.addstr(registrationResponse.toMessage());
        registrationReply.send(*mBackend);
    }
    /// @brief Handles the reading/processing of messages from the backend.
    void __handleMessagesFromBackend()
    {
        zmq::multipart_t messagesReceived(*mBackend);
        if (messagesReceived.empty()){return;} // Message somehow empty 
        if (messagesReceived.size() == 3)
        {
            RegistrationRequest registrationRequest;
            ::PingResponse pingResponse;
            ::TerminateResponse terminateResponse;
            ::PingRequest pingRequest;
            ::TerminateRequest terminateRequest;
            auto messageType = messagesReceived.at(1).to_string();
            if (messageType == registrationRequest.getMessageType())
            {
                __handleRegistrationRequest(messagesReceived);
            }
            else if (messageType == pingResponse.getMessageType())
            {
                __handlePingResponse(messagesReceived);
            }
            else if (messageType == terminateResponse.getMessageType())
            {
                __handleTerminateResponse(messagesReceived);
            }
            // Ignore these
            else if (messageType == pingRequest.getMessageType())
            {
            }
            // Ignore these
            else if (messageType == terminateRequest.getMessageType())
            {
            }
            else
            {
                mLogger->error("Unhandled message type: " + messageType);
            }
        }
        // Business as usual - propagate these back
        else if (messagesReceived.size() == 5)
        {
            // Purge the the first address (that's the server's).
            // Format is:
            // 1. Client Address
            // 2. Empty
            // 3. Message [Header+Body; this is actually len 4]
            messagesReceived.popstr();
            messagesReceived.send(*mFrontend);
        } // End check on non-empty message received
        else
        {
            mLogger->error("Unhandled message size: "
                         + std::to_string(messagesReceived.size()));
        }
    }

    /// @brief Starts the proxy.
    void start()
    {
        stop(); 
        setRunning(true);
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
        mProxyThread = std::thread(&ProxyImpl::runPoller, this);
        mModuleStatusThread = std::thread(&ProxyImpl::runModulePinger, this);
    }
    /// @brief Stops the proxy.
    void stop()
    {
        setRunning(false);
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
        if (mModuleStatusThread.joinable()){mModuleStatusThread.join();}
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
    /// @brief Update connection details.
    void updateConnectionDetails()
    {
        UCI::SocketDetails::Proxy socketDetails;
        auto securityLevel = mOptions.getZAPOptions().getSecurityLevel();
        UCI::SocketDetails::Router router;
        router.setAddress(mFrontendAddress);
        router.setSecurityLevel(securityLevel);
        router.setConnectOrBind(UCI::ConnectOrBind::Connect);

        UCI::SocketDetails::Router dealer;
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
    // If module is dead after this interval then it is purged from the list
    // Additionally, a terminate message is sent.
    std::chrono::milliseconds mModuleTimeOutInterval;
    // The registered modules
    ::ModulesMap mModulesMap;
    // Queue: ping responses (from proxy thread for module status thread)
    ::ThreadSafeQueue<std::pair<std::string, ::PingResponse>> mPingResponses;
    // Queue: ping requests (from module status thread to proxy thread to send)
    ::ThreadSafeQueue<std::pair<std::string, ::PingRequest>> mPingRequests;
    // Queue: terminate requests (from module status thread to proxy
    //        thread to send)
    ::ThreadSafeQueue<std::pair<std::string, ::TerminateRequest>>
        mTerminateRequests;
    ProxyOptions mOptions;
    UCI::Details mConnectionDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mMonitorAddress;
    const std::string mProxyName{"ModuleRegistry"};
    std::thread mAuthenticatorThread;
    std::thread mFrontendAuthenticatorThread;
    std::thread mBackendAuthenticatorThread;
    std::thread mProxyThread;
    std::thread mModuleStatusThread;
    std::vector<std::chrono::milliseconds>
        mPingIntervals{std::chrono::milliseconds {10000}};
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
    pImpl->mPingIntervals = options.getPingIntervals();
    pImpl->mModuleTimeOutInterval = pImpl->mPingIntervals.back()
                                  + std::chrono::milliseconds{100};
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
