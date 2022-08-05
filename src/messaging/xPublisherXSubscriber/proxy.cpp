#include <iostream>
#include <string>
#include <array>
#include <mutex>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/xPublisherXSubscriber/proxy.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/enums.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

namespace 
{

void checkOptions(const ProxyOptions &options)
{
    if (!options.haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not specified");
    }
    if (!options.haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not specified");
    } 
}

}

class Proxy::ProxyImpl
{
public:
    /*
    ProxyImpl(std::shared_ptr<zmq::context_t> context,
              std::shared_ptr<UMPS::Logging::ILog> logger, int) :
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
                                                     zmq::socket_type::xsub);
        mBackend = std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::xpub);
    }
    */
    ProxyImpl(std::shared_ptr<UMPS::Messaging::Context> context,
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
        // Now make the sockets
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mFrontend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                     zmq::socket_type::xsub);
        mBackend = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::xpub);
    }
    void setStarted(const bool status)
    {
       std::scoped_lock lock(mMutex);
       mStarted = status;
    }
    bool isStarted() const
    {
       std::scoped_lock lock(mMutex);
       return mStarted;
    }
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
    /// Bind the backend
    void bindBackend()
    {
        mBackendAddress = mOptions.getBackendAddress();
        try
        {
            mLogger->debug("Attempting to bind to backend: "
                         + mBackendAddress);
            mBackend->set(zmq::sockopt::linger, 0);
            int hwm = mOptions.getBackendHighWaterMark();
            if (hwm > 0){mBackend->set(zmq::sockopt::sndhwm, hwm);}
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
    void bindFrontend()
    {
        mFrontendAddress = mOptions.getFrontendAddress();
        try 
        {
            mLogger->debug("Attempting to bind to frontend: "
                         + mFrontendAddress);
            mFrontend->set(zmq::sockopt::linger, 0);
            int hwm = mOptions.getFrontendHighWaterMark();
            if (hwm > 0){mFrontend->set(zmq::sockopt::rcvhwm, hwm);}
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
    void connectControl()
    {
        // This is very unlikely to generate a collision.
        // Effectively the OS would have to overwrite this class's
        // location in memory.
        std::ostringstream address;
        address << static_cast<void const *> (this);
        auto nowMuSec = std::chrono::duration_cast<std::chrono::microseconds>
                  (std::chrono::system_clock::now().time_since_epoch()).count();
        mControlAddress = "inproc://"
                        + std::to_string(nowMuSec)
                        + " _" + address.str()
                        + "_xpubxsub_control";
        /*
        mControlAddress = "inproc://" + mOptions.getTopic()
                        + "_xpubxsub_control";
        */
        // Connect the control
        try
        {
            mLogger->debug("Attempting to bind to control: "
                         + mControlAddress);
            mControl->connect(mControlAddress);
            // The command will issue simple commands without topics so listen
            // to `everything.' 
            mControl->set(zmq::sockopt::subscribe, std::string(""));
            mControl->set(zmq::sockopt::linger, 1);
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
        UCI::SocketDetails::XSubscriber xSub;
        xSub.setAddress(mFrontendAddress);
        xSub.setSecurityLevel(mSecurityLevel);
        xSub.setConnectOrBind(UCI::ConnectOrBind::Connect);

        UCI::SocketDetails::XPublisher xPub;
        xPub.setAddress(mBackendAddress);
        xPub.setSecurityLevel(mSecurityLevel);
        xPub.setConnectOrBind(UCI::ConnectOrBind::Connect);
        mSocketDetails.setSocketPair(std::pair(xSub, xPub));
    }
///private:
    mutable std::mutex mMutex;
    // This context handles terminate/pause/start messages from the API
    std::unique_ptr<zmq::context_t> mControlContext{nullptr};
    // The control socket binds to the proxy and receives terminate/pause/start
    // messages from the command.
    std::unique_ptr<zmq::socket_t> mControl{nullptr};
    // The command socket issues terminate/pause/start messages to the control.
    std::unique_ptr<zmq::socket_t> mCommand{nullptr};
    // This context handles external communication with other servers
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    //std::shared_ptr<zmq::context_t> mContext{nullptr};
    // The front is an xsub that faces the internal servers
    std::unique_ptr<zmq::socket_t> mFrontend{nullptr};
    // The backend is an xpub that faces the external clients
    std::unique_ptr<zmq::socket_t> mBackend{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    // Options
    ProxyOptions mOptions;
    UCI::SocketDetails::Proxy mSocketDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mControlAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
    //int mHighWaterMark = 4*1024;
    bool mHaveFrontend = false;
    bool mHaveBackend = false;
    bool mHaveControl = false;
    bool mStarted = false;
    bool mPaused = false;
    bool mInitialized = false;
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> (nullptr, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, logger))
{
}

/*
Proxy::Proxy(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr, 0))
{
}
*/

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr))
{
}

/*
Proxy::Proxy(std::shared_ptr<zmq::context_t> &context,
             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (context, logger, 0))
{
}
*/

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (context, logger))
{
}

/// Setup the socket
void Proxy::initialize(const ProxyOptions &options)
{
    // Check and copy options
    checkOptions(options);
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

/// Initialized?
bool Proxy::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

std::string Proxy::getFrontendAddress() const
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    return pImpl->mFrontendAddress;
}

std::string Proxy::getBackendAddress() const
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    return pImpl->mBackendAddress;
}

/// Security level
UAuth::SecurityLevel Proxy::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Destructor
Proxy::~Proxy() = default;

// Starts the proxy
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
                auto errorMsg = "xPub/xSub proxy error.  ZeroMQ failed with:\n"
                              + std::string(e.what());
                pImpl->mLogger->error(errorMsg);
                throw std::runtime_error(errorMsg);
            }
        }
    }
}

/// Is running?
bool Proxy::isRunning() const noexcept
{
    return pImpl->isStarted();
}

// Pauses the proxy
void Proxy::pause()
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    if (!pImpl->mPaused)
    {
        pImpl->mLogger->debug("Pausing proxy...");
        pImpl->mCommand->send(zmq::str_buffer("PAUSE"), zmq::send_flags::none);
        pImpl->mPaused = true;
    }
}

// Stops the proxy
void Proxy::stop()
{
    if (pImpl->isStarted())
    {
        pImpl->mLogger->debug("Terminating proxy...");
        pImpl->mCommand->send(zmq::str_buffer("TERMINATE"),
                              zmq::send_flags::none);
        pImpl->disconnectFrontend();
        pImpl->disconnectBackend();
    }
    pImpl->mInitialized = false;
    pImpl->mStarted = false;
}

/// Socket details
UCI::SocketDetails::Proxy Proxy::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Proxy not initialized");
    }
    return pImpl->mSocketDetails;
}
