#include <string>
#include <array>
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
        mControlContext(std::make_unique<zmq::context_t> ()),
        mFrontend(std::make_unique<zmq::socket_t> (*mContext,
                                                   zmq::socket_type::xpub)),
        mBackend( std::make_unique<zmq::socket_t> (*mContext, 
                                                   zmq::socket_type::xsub)),
        mCapture( std::make_unique<zmq::socket_t> ()),
        mControl( std::make_unique<zmq::socket_t> (*mControlContext,
                                                   zmq::socket_type::sub)),
        mLogger(std::make_shared<URTS::Logging::StdOut> ())
    {
//        mControl->set(ZMQ_SUBSCRIBE, "", 0);
    }
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::context_t> mControlContext = nullptr;
    //std::unique_ptr<zmq::proxy_steerable> mProxy = nullptr;
    std::unique_ptr<zmq::socket_t> mFrontend = nullptr;
    std::unique_ptr<zmq::socket_t> mBackend = nullptr;
    std::unique_ptr<zmq::socket_t> mCapture = nullptr;
    std::unique_ptr<zmq::socket_t> mControl = nullptr;
    std::shared_ptr<URTS::Logging::ILog> mLogger = nullptr;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    std::string mTopic;
    std::string mControlAddress;
    bool mHaveFrontend = false;
    bool mHaveBackend = false;
    bool mStarted = false;
    bool mPaused = false;
    bool mInitialized = false;
/*
proxy_steerable(socket_ref frontend,
                            socket_ref backend,
                            socket_ref capture,
                            socket_ref control) 
*/
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> ())
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
    pImpl->mTopic = topic;
    pImpl->mControlAddress = "inproc://" + topic + "_control";
    // Disconnect from old connections
    if (pImpl->mHaveFrontend)
    {
        pImpl->mFrontend->disconnect(pImpl->mFrontendAddress);
        pImpl->mHaveFrontend = false;
    }
    if (pImpl->mHaveBackend)
    {
        pImpl->mBackend->disconnect(pImpl->mBackendAddress);
        pImpl->mHaveBackend = false;
    }
    pImpl->mFrontendAddress = frontendAddress;
    pImpl->mBackendAddress = backendAddress;
    // Bind the frontend
    try
    {
        pImpl->mFrontend->bind(pImpl->mFrontendAddress);
        pImpl->mHaveFrontend = true;
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Proxy failed to bind to frontend: " + frontendAddress
                      + ".  ZeroMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    // Bind the backend
    try
    {
        pImpl->mBackend->bind(pImpl->mBackendAddress);
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
        pImpl->mControl->connect(pImpl->mControlAddress);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Proxy failed to bind to control: "
                      + pImpl->mControlAddress
                      + ".  ZeroMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
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
        pImpl->mControl->send(zmq::str_buffer("RESUME"), zmq::send_flags::none);
        pImpl->mPaused = false;
    }
    else
    {
        if (!pImpl->mStarted)
        {
            try
            {
                zmq::proxy_steerable(zmq::socket_ref(*pImpl->mFrontend),
                                     zmq::socket_ref(*pImpl->mBackend),
                                     zmq::socket_ref(*pImpl->mCapture),
                                     zmq::socket_ref(*pImpl->mControl));
                pImpl->mStarted = true;
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
    pImpl->mStarted = true;
}

// Pauses the proxy
void Proxy::pause()
{
    if (!isInitialized()){throw std::runtime_error("Proxy not initialized");}
    if (!pImpl->mPaused)
    {
        pImpl->mControl->send(zmq::str_buffer("PAUSE"), zmq::send_flags::none);
        pImpl->mPaused = true;
    }
}

// Stops the proxy
void Proxy::stop()
{
    if (pImpl->mStarted)
    {
        pImpl->mControl->send(zmq::str_buffer("TERMINATE"), 
                              zmq::send_flags::none);
        if (pImpl->mHaveFrontend)
        {
            pImpl->mFrontend->disconnect(pImpl->mFrontendAddress);
            pImpl->mHaveFrontend = false;
        }
        if (pImpl->mHaveBackend)
        {
            pImpl->mBackend->disconnect(pImpl->mBackendAddress);
            pImpl->mHaveBackend = false;
        }
    }
    pImpl->mInitialized = false;
    pImpl->mStarted = false;
}
