#include <mutex>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/standardOut.hpp"
#include "private/messaging/ipcDirectory.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Proxy::ProxyImpl
{
public:
    /// C'tor
    ProxyImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
              const std::shared_ptr<UMPS::Logging::ILog> &logger)
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
        // Now make the sockets
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mFrontend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                     zmq::socket_type::router);
        mFrontend->set(zmq::sockopt::router_mandatory, 1); 
        mBackend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::dealer);
    } 
    ///Destructor
    ~ProxyImpl()
    {   
        disconnectFrontend();
        disconnectBackend();
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
    /// Bind the frontend
    void bindFrontend()
    {
        mFrontendAddress = mOptions.getFrontendAddress();
        // Resolve a directory issue for IPC
        ::createIPCDirectoryFromConnectionString(mFrontendAddress, &*mLogger);
        try
        {
            mLogger->debug("Proxy attempting to bind to frontend: "
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
            auto errorMsg = "Proxy failed to bind to frontend: "
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
            mLogger->debug("Proxy attempting to bind to backend: "
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
            auto errorMsg = "Proxy failed to bind to backend: "
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
    /// Update socket details
    void updateSocketDetails()
    {
        UCI::SocketDetails::Router router;
        UCI::SocketDetails::Dealer dealer;
        router.setAddress(mFrontendAddress);
        router.setSecurityLevel(mSecurityLevel);
        router.setConnectOrBind(UCI::ConnectOrBind::Connect);
        dealer.setAddress(mBackendAddress);
        dealer.setSecurityLevel(mSecurityLevel);
        dealer.setConnectOrBind(UCI::ConnectOrBind::Connect);
        mSocketDetails.setSocketPair(std::pair(router, dealer));
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mFrontend{nullptr};
    std::unique_ptr<zmq::socket_t> mBackend{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    ProxyOptions mOptions;
    UCI::SocketDetails::Proxy mSocketDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mControlAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::Grasslands;
    std::chrono::milliseconds mPollTimeOutMS{10};
    bool mHaveFrontend = false;
    bool mHaveBackend = false;
    bool mHaveControl = false;
    bool mRunning = false;
    bool mPaused = false;
    bool mInitialized = false;
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> (nullptr, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr)) 
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (context, logger)) 
{
}

/// Initialize
void Proxy::initialize(const ProxyOptions &options)
{
    // Check and copy options
    if (!options.haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    if (!options.haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    pImpl->mOptions = options; 
    pImpl->mInitialized = false;
    // Disconnect from old connections
    pImpl->mSocketDetails.clear();
    pImpl->disconnectFrontend();
    pImpl->disconnectBackend();
    // Create zap options
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    zapOptions.setSocketOptions(&*pImpl->mFrontend);
    zapOptions.setSocketOptions(&*pImpl->mBackend);
    // (Re)Establish connections
    pImpl->bindBackend();
    pImpl->bindFrontend();
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    pImpl->updateSocketDetails();
    pImpl->mInitialized = true;
}

void Proxy::start()
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
    while (isRunning())
    {
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS); 
        if (items[0].revents & ZMQ_POLLIN)
        {
            // Forward the next messages
            try
            {
                zmq::multipart_t messagesReceived(*pImpl->mFrontend);
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
            catch (const std::exception &e)
            {
                auto errorMsg = "Frontend to backend proxy std error:  "
                              + std::string(e.what());
                pImpl->mLogger->error(errorMsg);
            }
        }
        if (items[1].revents & ZMQ_POLLIN)
        {
            try
            {
                zmq::multipart_t messagesReceived(*pImpl->mBackend);
                messagesReceived.send(*pImpl->mFrontend);
            }
            catch (const zmq::error_t &e)
            {
                auto errorMsg = "Backend to frontend proxy error.  "
                              + std::string("ZeroMQ failed with:\n")
                              + std::string(e.what())
                              + " Error Code = " + std::to_string(e.num());
                pImpl->mLogger->error(errorMsg);
            }
            catch (const std::exception &e) 
            {
                auto errorMsg = "Backend to frontend proxy std error:  "
                              + std::string(e.what());
                pImpl->mLogger->error(errorMsg);
            }
        }
    }
}

/*
/// Run the proxy
void Proxy::start()
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    if (pImpl->mPaused)
    {
        pImpl->mLogger->debug("Resuming proxy...");
        pImpl->mCommand->send(zmq::str_buffer("RESUME"), zmq::send_flags::none);
        pImpl->mPaused = false;
    }
    else
    {
        if (!isRunning())
        {
            try
            {
                pImpl->mLogger->debug("Making steerable proxy...");
                pImpl->setStarted(true);
                zmq::proxy_steerable(*pImpl->mFrontend,
                                     *pImpl->mBackend,
                                     zmq::socket_ref(),
                                     *pImpl->mControl);
                pImpl->mLogger->debug("Exiting steerable proxy");
                pImpl->setStarted(false);
            }
            catch (const zmq::error_t &e) //std::exception &e) 
            {
                auto errorMsg = "Router/dealer proxy error.  ZeroMQ failed with:\n"
                              + std::string(e.what())
                              + " Error Code = " + std::to_string(e.num());
                std::cerr << errorMsg << std::endl;
                pImpl->mLogger->error(errorMsg);
                try
                {
                    pImpl->disconnectBackend();
                    pImpl->disconnectFrontend();
                    pImpl->bindBackend();
                    pImpl->bindFrontend();
                    pImpl->updateSocketDetails();
                    pImpl->setStarted(false);
                }
                catch (const std::exception &e)
                {
                    throw std::runtime_error(errorMsg);
                }
            }   
        }   
    }   
}
*/

/// Initialized?
bool Proxy::isInitialized() const noexcept
{
    return pImpl->mInitialized;
} 

/// Destructor
Proxy::~Proxy() = default;

/// Is running?
bool Proxy::isRunning() const noexcept
{
    return pImpl->isRunning();
}

void Proxy::stop()
{
    if (isRunning())
    {
        pImpl->mLogger->debug("Terminating proxy...");
        pImpl->setStarted(false);
        pImpl->disconnectFrontend();
        pImpl->disconnectBackend();
    }
    pImpl->mInitialized = false;
    pImpl->setStarted(false);
}

/*
// Stops the proxy
void Proxy::stop()
{
    if (isRunning())
    {   
        pImpl->mLogger->debug("Terminating proxy...");
        pImpl->mCommand->send(zmq::str_buffer("TERMINATE"),
                              zmq::send_flags::none);
        pImpl->disconnectFrontend();
        pImpl->disconnectBackend();
    }   
    pImpl->mInitialized = false;
    pImpl->setStarted(false);
}
*/

/// Connection details
UCI::SocketDetails::Proxy Proxy::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Proxy not initialized");
    }
    return pImpl->mSocketDetails;
}

/// Security level
UAuth::SecurityLevel Proxy::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

