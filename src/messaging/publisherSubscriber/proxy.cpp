#include <iostream>
#include <string>
#include <array>
#include <mutex>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "urts/messaging/publisherSubscriber/proxy.hpp"
#include "urts/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace URTS::Messaging::PublisherSubscriber;

class Proxy::ProxyImpl
{
public:
    ProxyImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mControlContext(std::make_unique<zmq::context_t> (0)),
        mFrontend(std::make_unique<zmq::socket_t> (*mContext,
                                                   zmq::socket_type::xsub)),
        mBackend( std::make_unique<zmq::socket_t> (*mContext, 
                                                   zmq::socket_type::xpub)),
        mControl( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::sub)),
        mCommand( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::pub)),
        mLogger(std::make_shared<URTS::Logging::StdOut> ())
    {
    }
    explicit ProxyImpl(std::shared_ptr<URTS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mControlContext(std::make_unique<zmq::context_t> (0)),
        mFrontend(std::make_unique<zmq::socket_t> (*mContext,
                                                   zmq::socket_type::xsub)),
        mBackend( std::make_unique<zmq::socket_t> (*mContext, 
                                                   zmq::socket_type::xpub)),
        mControl( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::sub)),
        mCommand( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::pub)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<URTS::Logging::StdOut> ();
        }
    }
    void setStarted(const bool status)
    {
       std::scoped_lock lock(mMutex);
       mStarted = status;
    }
    bool isStarted() const
    {
       std::scoped_lock lock(mMutex);
       auto started = mStarted;
       return started;
    }
    mutable std::mutex mMutex;
    // This context handles external communication with other servers
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    // This context handles terminate/pause/start messages from the API
    std::unique_ptr<zmq::context_t> mControlContext = nullptr;
    // The front is an xsub that faces the internal servers
    std::unique_ptr<zmq::socket_t> mFrontend = nullptr;
    // The backend is an xpub that faces the external clients
    std::unique_ptr<zmq::socket_t> mBackend = nullptr;
    // The control socket binds to the proxy and receives terminate/pause/start
    // messages from the command.
    std::unique_ptr<zmq::socket_t> mControl = nullptr;
    // The command socket issues terminate/pause/start messages to the control.
    std::unique_ptr<zmq::socket_t> mCommand = nullptr;
    std::shared_ptr<URTS::Logging::ILog> mLogger = nullptr;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mTopic;
    std::string mControlAddress;
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
    pImpl(std::make_unique<ProxyImpl> ())
{
}

Proxy::Proxy(std::shared_ptr<URTS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProxyImpl> (logger))
{
}

/// Move c'tor
Proxy::Proxy(Proxy &&proxy) noexcept
{
    *this = std::move(proxy);
}

/// Move assignment
Proxy& Proxy::operator=(Proxy &&proxy) noexcept
{
    if (&proxy == this){return *this;}
    pImpl = std::move(proxy.pImpl);
    return *this;
}

/// Initialize the proxy
void Proxy::initialize(const std::string &frontendAddress,
                       const std::string &backendAddress,
                       const std::string &topic)
{
    if (isEmpty(frontendAddress))
    {
        throw std::invalid_argument("Frontend address is blank");
    }
    if (isEmpty(backendAddress))
    {
        throw std::invalid_argument("Backend address is blank");
    }
    if (isEmpty(topic))
    {
        throw std::invalid_argument("Topic is blank");
    }
    pImpl->mInitialized = false;
    // Disconnect from old connections
    if (pImpl->mHaveFrontend)
    {
        pImpl->mLogger->debug("Disconnecting from current frontend: "
                            + pImpl->mFrontendAddress);
        pImpl->mFrontend->disconnect(pImpl->mFrontendAddress);
        pImpl->mHaveFrontend = false;
    }
    if (pImpl->mHaveBackend)
    {
        pImpl->mLogger->debug("Disconnecting from current backend: "
                            + pImpl->mBackendAddress);
        pImpl->mBackend->disconnect(pImpl->mBackendAddress);
        pImpl->mHaveBackend = false;
    }
    if (pImpl->mHaveControl)
    {
        pImpl->mLogger->debug("Disconnecting from current control: "
                            + pImpl->mControlAddress);
        pImpl->mControl->disconnect(pImpl->mControlAddress);
        pImpl->mHaveControl = false;
    }
    pImpl->mFrontendAddress = frontendAddress;
    pImpl->mBackendAddress = backendAddress;
    pImpl->mControlAddress = "inproc://" + topic + "_control";
    // Connect the frontend
    try
    {
        pImpl->mLogger->debug("Attempting to connect to frontend: "
                            + pImpl->mFrontendAddress);
        pImpl->mFrontend->connect(pImpl->mFrontendAddress);
        //pImpl->mFrontend->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
        pImpl->mHaveFrontend = true;
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Proxy failed to connect to frontend: "
                      + frontendAddress + ".  ZeroMQ failed with:\n"
                      + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    // Bind the backend
    try
    {
        pImpl->mLogger->debug("Attempting to bind to backend: "
                            + pImpl->mBackendAddress);
        pImpl->mBackend->bind(pImpl->mBackendAddress);
        pImpl->mHaveBackend = true;
    }
    catch (const std::exception &e) 
    {   
        auto errorMsg = "Proxy failed to bind to backend: " + backendAddress
                      + ".  ZeroMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    // Bind the control
    try
    {
        pImpl->mLogger->debug("Attempting to bind to control: "
                            + pImpl->mControlAddress);
        pImpl->mControl->connect(pImpl->mControlAddress);
        // The command will issue simple commands without topics so listen
        // to `everything.' 
        pImpl->mControl->set(zmq::sockopt::subscribe, std::string(""));
        pImpl->mCommand->bind(pImpl->mControlAddress);
        pImpl->mHaveControl = true;
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Proxy failed to bind to control: "
                      + pImpl->mControlAddress
                      + ".  ZeroMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    pImpl->mTopic = topic;
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
        if (!pImpl->mStarted)
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
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");} 
    if (pImpl->isStarted())
    {
        pImpl->mLogger->debug("Terminating proxy...");
        pImpl->mCommand->send(zmq::str_buffer("TERMINATE"),
                              zmq::send_flags::none);
        if (pImpl->mHaveFrontend)
        {
            pImpl->mLogger->debug("Disconnecting from frontend");
            pImpl->mFrontend->disconnect(pImpl->mFrontendAddress);
            pImpl->mHaveFrontend = false;
        }
        if (pImpl->mHaveBackend)
        {
            pImpl->mLogger->debug("Disconnecting from backend");
            pImpl->mBackend->disconnect(pImpl->mBackendAddress);
            pImpl->mHaveBackend = false;
        }
    }
    pImpl->mInitialized = false;
    pImpl->mStarted = false;
}