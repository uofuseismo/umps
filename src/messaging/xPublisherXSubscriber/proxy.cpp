#include <string>
#include <thread>
#include <array>
#include <mutex>
#include <chrono>
#include <zmq.hpp>
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
#include "umps/logging/standardOut.hpp"
#include "private/messaging/ipcDirectory.hpp"

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
    ProxyImpl(const std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
              const std::shared_ptr<UMPS::Messaging::Context> &backendContext,
              const std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mControlContext(std::make_unique<zmq::context_t> (0)),
        mControl( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::rep)), // sub
        mCommand( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::req)) // pub
    {
        // Ensure the context gets made
        if (frontendContext == nullptr &&
            backendContext == nullptr)
        {
            mFrontendContext = std::make_shared<UMPS::Messaging::Context> (1);
            mBackendContext = mFrontendContext;
        }
        else
        {
            if (frontendContext == nullptr)
            {
                mFrontendContext
                    = std::make_shared<UMPS::Messaging::Context> (1);
            }
            else
            {
                mFrontendContext = frontendContext;
            }
            if (backendContext == nullptr)
            {
                mBackendContext
                    = std::make_shared<UMPS::Messaging::Context> (1);
            }
            else
            {
                mBackendContext = backendContext;
            }
        }
#ifndef NDEBUG
        assert(mFrontendContext != nullptr);
        assert(mBackendContext !=  nullptr);
#endif
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
        auto frontendContextPtr = reinterpret_cast<zmq::context_t *>
                                  (mFrontendContext->getContext());
        auto backendContextPtr = reinterpret_cast<zmq::context_t *>
                                 (mBackendContext->getContext()); 
        mFrontend = std::make_unique<zmq::socket_t> (*frontendContextPtr,
                                                     zmq::socket_type::xsub);
        mBackend = std::make_unique<zmq::socket_t> (*backendContextPtr,
                                                    zmq::socket_type::xpub);
    }
    /// Destructor
    ~ProxyImpl()
    {
        disconnectFrontend();
        disconnectBackend();
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
            ::removeIPCFile(mFrontendAddress, &*mLogger);
            mLogger->debug(
                "xPubxSub proxy disconnecting from current frontend: "
               + mFrontendAddress);
            mFrontend->disconnect(mFrontendAddress);
            mLogger->debug("xPubxSub disconnected frontend");
            mHaveFrontend = false;
        }
    }
    void disconnectBackend()
    {
        if (mHaveBackend)
        {
            ::removeIPCFile(mBackendAddress, &*mLogger);
            mLogger->debug(
                "xPubxSub disconnecting from current backend: "
               + mBackendAddress);
            mBackend->disconnect(mBackendAddress);
            mLogger->debug("xPubxSub disconnected from backend");
            mHaveBackend = false;
        }
    }
    void disconnectControl()
    {
        if (mHaveControl)
        {
            mLogger->debug(
                "xPubxSub disconnecting from current control: "
               + mControlAddress);
            mControl->disconnect(mControlAddress);
            mLogger->debug("xPubxSub disconnected control");
            mHaveControl = false;
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
            mLogger->debug("xPubSubProxy proxy attempting to bind to backend: "
                         + mBackendAddress);
            mBackend->set(zmq::sockopt::linger, 0);
            int hwm = mOptions.getBackendHighWaterMark();
            if (hwm > 0){mBackend->set(zmq::sockopt::sndhwm, hwm);}
            mBackend->bind(mBackendAddress);
            mHaveBackend = true;
        }
        catch (const std::exception &e)
        {
            auto errorMsg = "xPubSubProxy proxy failed to bind to backend: "
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
        // Resolve a directory issue for IPC
        ::createIPCDirectoryFromConnectionString(mFrontendAddress, &*mLogger);
        try 
        {
            mLogger->debug("xPubSubProxy proxy attempting to bind to frontend: "
                         + mFrontendAddress);
            mFrontend->set(zmq::sockopt::linger, 0);
            int hwm = mOptions.getFrontendHighWaterMark();
            if (hwm > 0){mFrontend->set(zmq::sockopt::rcvhwm, hwm);}
            mFrontend->bind(mFrontendAddress);
            mHaveFrontend = true;
        }
        catch (const std::exception &e)
        {
            auto errorMsg = "xPubSubProxy proxy failed to bind to frontend: "
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
                        + "_" + address.str()
                        + "_xpubxsub_control";
        // Connect the control
        try
        {
            mLogger->debug("Attempting to bind to control: "
                         + mControlAddress);
            mControl->bind(mControlAddress);
            // The command will issue simple commands without topics so listen
            // to everything.
            //mControl->set(zmq::sockopt::subscribe, std::string(""));
            //mControl->set(zmq::sockopt::linger, 1);
            mCommand->set(zmq::sockopt::linger, 0);
            mCommand->connect(mControlAddress); // Connects instead of binds?
            mHaveControl = true;
        }
        catch (const std::exception &e)
        {
            auto errorMsg = "Proxy failed to bind to control: "
                           + mControlAddress
                          + ".  ZeroMQ failed with: " + std::string(e.what());
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
        mSocketDetails.setSocketPair(std::pair{xSub, xPub});
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
    // This context handles communication with producers.
    std::shared_ptr<UMPS::Messaging::Context> mFrontendContext{nullptr};
    // This context handles communication with the subscribers.
    std::shared_ptr<UMPS::Messaging::Context> mBackendContext{nullptr};
    // The frontend is an xSub that faces the internal servers
    std::unique_ptr<zmq::socket_t> mFrontend{nullptr};
    // The backend is an xPub that faces the external clients
    std::unique_ptr<zmq::socket_t> mBackend{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    // Options
    ProxyOptions mOptions;
    UCI::SocketDetails::Proxy mSocketDetails;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mControlAddress;
    UAuth::SecurityLevel mSecurityLevel{UAuth::SecurityLevel::Grasslands};
    //int mHighWaterMark = 4*1024;
    bool mHaveFrontend{false};
    bool mHaveBackend{false};
    bool mHaveControl{false};
    bool mStarted{false};
    bool mPaused{false};
    bool mInitialized{false};
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> (nullptr, nullptr, nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (nullptr, nullptr, logger))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr,  nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
             std::shared_ptr<UMPS::Messaging::Context> &backendContext) :
    pImpl(std::make_unique<ProxyImpl> (frontendContext,
                                       backendContext,
                                       nullptr))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (context, nullptr, logger))
{
}

Proxy::Proxy(std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
             std::shared_ptr<UMPS::Messaging::Context> &backendContext,
             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (frontendContext,
                                       backendContext,
                                       logger))
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
                pImpl->mLogger->debug("Exited steerable proxy");
                pImpl->disconnectControl();
                pImpl->mLogger->debug("Control disconnected");
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
        try
        {
            pImpl->mCommand->send(zmq::str_buffer("TERMINATE"),
                                  zmq::send_flags::none);
        }
        catch (const std::exception &e)
        {
            pImpl->mLogger->error("Failed to TERMINATE proxy.  Failed with: "
                                + std::string {e.what()});
        }
        try
        {
            pImpl->disconnectFrontend();
        }
        catch (const std::exception &e)
        {
            pImpl->mLogger->error(
                "Failed to disconnect frontend.  Failed with: "
               + std::string {e.what()});
        }
        try
        {
            pImpl->disconnectBackend();
        }
        catch (const std::exception &e)
        {
            pImpl->mLogger->error(
                "Failed to disconnect backend.  Failed with: "
               + std::string {e.what()});
        }
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
