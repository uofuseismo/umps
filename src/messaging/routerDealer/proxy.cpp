#include <iostream>
#include <mutex>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Proxy::ProxyImpl
{
public:
    ProxyImpl(std::shared_ptr<zmq::context_t> context,
              std::shared_ptr<UMPS::Logging::ILog> logger) :
        mControlContext(std::make_unique<zmq::context_t> (0)),
        mControl( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::sub)),
        mCommand( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::pub))
    {
        // Ensure the context gets made
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
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
        // Now make the sockets
        mFrontend = std::make_unique<zmq::socket_t> (*mContext,
                                                     zmq::socket_type::router);
        mBackend = std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::dealer);
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
        try
        {
            mLogger->debug("Attempting to bind to frontend: "
                         + mFrontendAddress);
            mFrontend->bind(mFrontendAddress);
            int hwm = mOptions.getFrontendHighWaterMark();
            if (hwm > 0)
            {
                mFrontend->set(zmq::sockopt::sndhwm, hwm);
                mFrontend->set(zmq::sockopt::rcvhwm, hwm);
            }
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
        try
        {
            mLogger->debug("Attempting to bind to backend: "
                         + mBackendAddress);
            mBackend->bind(mBackendAddress);
            int hwm = mOptions.getBackendHighWaterMark();
            if (hwm > 0)
            {
                mBackend->set(zmq::sockopt::sndhwm, hwm);
                mBackend->set(zmq::sockopt::rcvhwm, hwm);
            }
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
            mLogger->debug("Disconnecting from current backend: "
                         + mBackendAddress);
            mBackend->disconnect(mBackendAddress);
            mHaveBackend = false;
        }   
    }
    /// Disconnect control
    void disconnectControl()
    {
        if (mHaveControl)
        {
            mLogger->debug("Disconnecting from current control: "
                         + mControlAddress);
            mControl->disconnect(mControlAddress);
            mHaveControl = false;
        }
    }
    /// Connect control
    void connectControl()
    {
        // This is very unlikely to generate a collision
        std::ostringstream address;
        address << static_cast<void const *> (this);
        auto nowMuSec = std::chrono::duration_cast<std::chrono::microseconds>
                  (std::chrono::system_clock::now().time_since_epoch()).count();
        mControlAddress = "inproc://"
                        + std::to_string(nowMuSec)
                        + " _" + address.str()
                        + "_routerdealer_control";
        try
        {
            mLogger->debug("Attempting to bind to control: "
                         + mControlAddress);
            mControl->connect(mControlAddress);
            // The command will issue simple commands without topics so listen
            // to `everything.' 
            mControl->set(zmq::sockopt::subscribe, std::string(""));
            mCommand->bind(mControlAddress);
            mHaveControl = true;
        }
        catch (const std::exception &e) 
        {
            auto errorMsg = "Proxy failed to bind to control: "
                           + mControlAddress
                          + ".\nZeroMQ failed with:\n" + std::string(e.what());
            mLogger->error(errorMsg);
            throw std::runtime_error(errorMsg);
        }
    }
    /// Update socket details
    void updateSocketDetails()
    {
        UCI::SocketDetails::Router router;
        UCI::SocketDetails::Dealer dealer;
        router.setAddress(mFrontendAddress);
        router.setSecurityLevel(mSecurityLevel);
        router.setConnectOrBind(UCI::ConnectOrBind::CONNECT);
        dealer.setAddress(mBackendAddress);
        dealer.setSecurityLevel(mSecurityLevel);
        dealer.setConnectOrBind(UCI::ConnectOrBind::CONNECT);
        mSocketDetails.setSocketPair(std::pair(router, dealer));
    }
///private:
    mutable std::mutex mMutex;
    std::unique_ptr<zmq::context_t> mControlContext;
    std::unique_ptr<zmq::socket_t> mControl;
    std::unique_ptr<zmq::socket_t> mCommand;

    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mFrontend;
    std::unique_ptr<zmq::socket_t> mBackend;

    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    ProxyOptions mOptions;
    UCI::SocketDetails::Proxy mSocketDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mControlAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
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

Proxy::Proxy(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger))
{
}

Proxy::Proxy(std::shared_ptr<zmq::context_t> &context,
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
    pImpl->disconnectControl();
    // Create zap options
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    zapOptions.setSocketOptions(&*pImpl->mFrontend);
    zapOptions.setSocketOptions(&*pImpl->mBackend);
    // (Re)Establish connections
    pImpl->bindBackend();
    pImpl->bindFrontend();
    pImpl->connectControl();
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    pImpl->updateSocketDetails();
    pImpl->mInitialized = true;
}

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
                pImpl->disconnectControl();
                pImpl->setStarted(false);
            }
            catch (const std::exception &e) 
            {
                auto errorMsg = "Failed to start proxy.  ZeroMQ failed with:\n"
                              + std::string(e.what());
                pImpl->mLogger->error(errorMsg);
                throw std::runtime_error(errorMsg);
            }   
        }   
    }   
}

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

