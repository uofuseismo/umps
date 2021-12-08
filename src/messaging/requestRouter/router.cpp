#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RequestRouter;

namespace UAuth = UMPS::Messaging::Authentication;

class Router::RouterImpl
{
public:
    /*
    /// C'tor
    RouterImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mServer(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::router)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    /// C'tor with specified logger
    explicit RouterImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mServer(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::router)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    */
    /// C'tor
    RouterImpl(std::shared_ptr<zmq::context_t> context,
               std::shared_ptr<UMPS::Logging::ILog> logger)
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
        // Now make the socket
        mServer = std::make_unique<zmq::socket_t> (*mContext,
                                                   zmq::socket_type::router);
    }
    /// Start the service
    void start()
    {
        std::scoped_lock lock(mMutex);
        mRunning = true;
    }
    /// Stop the service
    void stop()
    {
        std::scoped_lock lock(mMutex);
        mRunning = false;
    }
    /// Determines if the service was started
    bool isRunning() const noexcept
    {   
        std::scoped_lock lock(mMutex);
        auto running = mRunning;
        return running;
    }
    // Unbind
    void unbind()
    {
       std::scoped_lock lock(mMutex);
       if (mBound)
       {
           mServer->disconnect(mEndPoint);
           mEndPoint.clear();
           mBound = false;
       }
    }

//    std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
        //mSubscriptions;
    UMPS::MessageFormats::Messages mMessageFormats;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mServer;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    std::function<
          std::unique_ptr<UMPS::MessageFormats::IMessage>
          (const std::string &messageType, const void *contents,
           const size_t length)
    > mCallback;
    RouterOptions mOptions;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{10};
    mutable std::mutex mMutex;
    std::string mEndPoint;
    int mHighWaterMark = 100; 
    Authentication::SecurityLevel mSecurityLevel
        = Authentication::SecurityLevel::GRASSLANDS;
    bool mBound = false;
    bool mRunning = false; 
    bool mConnected = false;
    bool mHaveCallback = false;
    bool mInitialized = false;
};

/// C'tor
Router::Router() :
    pImpl(std::make_unique<RouterImpl> (nullptr, nullptr))
{
}


Router::Router(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RouterImpl> (nullptr, logger))
{
}

Router::Router(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<RouterImpl> (context, nullptr))
{
}


Router::Router(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RouterImpl> (context, logger))
{
}

/// Destructor
Router::~Router() = default;

//void Router::initialize(const std::string &endPoint,
/*
void Router::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mCallback = callback;
    pImpl->mHaveCallback = true;
}

bool Router::haveCallback() const noexcept
{
    return pImpl->mHaveCallback;
}
*/

/// Initialized?
bool Router::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Initializes the router
void Router::initialize(const RouterOptions &options)
{
    if (!options.haveEndPoint())
    {
        throw std::invalid_argument("End point not set");
    }
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
    pImpl->mMessageFormats = options.getMessageFormats();
    if (pImpl->mMessageFormats.empty())
    {
        pImpl->mLogger->warn("No message types set in options");
    }

    pImpl->mOptions.clear();
    pImpl->unbind();
    pImpl->mOptions = options;
    //  
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    auto highWaterMark = pImpl->mOptions.getHighWaterMark();
    auto endPoint = pImpl->mOptions.getEndPoint();
    // Set the ZAP options
    zapOptions.setSocketOptions(&*pImpl->mServer);
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    // Set the high water mark
    pImpl->mServer->set(zmq::sockopt::rcvhwm, highWaterMark);
    pImpl->mServer->set(zmq::sockopt::sndhwm, highWaterMark); 
    // Set the callback 
    pImpl->mCallback = pImpl->mOptions.getCallback(); 
    pImpl->mHaveCallback = true;
    // Bind
    pImpl->mServer->bind(endPoint);
    // Resolve end point
    pImpl->mEndPoint = endPoint;
    if (endPoint.find("tcp") != std::string::npos ||
        endPoint.find("ipc") != std::string::npos)
    {
        pImpl->mEndPoint = pImpl->mServer->get(zmq::sockopt::last_endpoint);
    }
    pImpl->mBound = true;
    pImpl->mInitialized = true;
}

/*
/// Bind to an address if not already done so
void Router::bind(const std::string &endPoint)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("Endpoint is empty");}
    pImpl->unbind();
    try
    {
        pImpl->mServer->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
        pImpl->mServer->set(zmq::sockopt::sndhwm, pImpl->mHighWaterMark);
        pImpl->mServer->bind(endPoint);
    }
    catch (const std::exception &e) 
    {
        auto errorMsg = "Failed to connect to endpoint: " + endPoint
                      + ".  ZeroMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    pImpl->mSecurityLevel = Authentication::SecurityLevel::GRASSLANDS;
    pImpl->mEndPoint = endPoint;
    pImpl->mBound = true;
}

/// Bind to an address - strawhouse 
void Router::bind(const std::string &endPoint,
                  const bool isAuthenticationServer,
                  const std::string &zapDomain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    pImpl->unbind(); // Hangup
    // ZAP 
    setStrawhouse(pImpl->mServer.get(), isAuthenticationServer, zapDomain);
    // Options
    pImpl->mServer->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
    pImpl->mServer->set(zmq::sockopt::sndhwm, pImpl->mHighWaterMark);
    // Bind
    pImpl->mServer->bind(endPoint);
  
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STRAWHOUSE;
    pImpl->mEndPoint = endPoint;
    pImpl->mBound = true;
}

/// Bind to an address - woodhouse
void Router::bind(
    const std::string &endPoint,
    const Authentication::Certificate::UserNameAndPassword &credentials,
    const bool isAuthenticationServer,
    const std::string &zapDomain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!isAuthenticationServer)
    {   
        if (!credentials.haveUserName())
        {
            throw std::invalid_argument("Username must be set for ZAP client");
        }
        if (!credentials.havePassword())
        {
            throw std::invalid_argument("Password must be set for ZAP client");
        }
    }
    pImpl->unbind(); // Hangup
    // ZAP 
    setWoodhouse(pImpl->mServer.get(), credentials,
                 isAuthenticationServer, zapDomain);
    // Options
    pImpl->mServer->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
    pImpl->mServer->set(zmq::sockopt::sndhwm, pImpl->mHighWaterMark);
    // Bind
    pImpl->mServer->bind(endPoint);

    pImpl->mSecurityLevel = Authentication::SecurityLevel::WOODHOUSE;
    pImpl->mEndPoint = endPoint;
    pImpl->mBound = true;
}

/// Bind to an address
void Router::bind(
    const std::string &endPoint,
    const Authentication::Certificate::Keys &serverKeys,
    const std::string &zapDomain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!serverKeys.havePrivateKey())
    {
        throw std::invalid_argument("Server private key not set");
    }
    pImpl->unbind(); // Hangup
    // Set ZAP
    setStonehouseServer(pImpl->mServer.get(), serverKeys, zapDomain);
    // Options
    pImpl->mServer->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
    pImpl->mServer->set(zmq::sockopt::sndhwm, pImpl->mHighWaterMark);
    // Bind
    pImpl->mServer->bind(endPoint);

    pImpl->mSecurityLevel = Authentication::SecurityLevel::WOODHOUSE;
    pImpl->mEndPoint = endPoint;
    pImpl->mBound = true;
}

/// Bind to an address
void Router::bind(
    const std::string &endPoint,
    const Authentication::Certificate::Keys &serverKeys,
    const Authentication::Certificate::Keys &clientKeys,
    const std::string &zapDomain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!clientKeys.havePublicKey())
    {
        throw std::invalid_argument("Client public key not set");
    }
    if (!clientKeys.havePrivateKey())
    {
        throw std::invalid_argument("Client private key not set");
    }
    pImpl->unbind(); // Hangup
    // Set ZAP protocol
    setStonehouseClient(pImpl->mServer.get(), serverKeys,
                       clientKeys, zapDomain);
    // Options
    pImpl->mServer->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
    pImpl->mServer->set(zmq::sockopt::sndhwm, pImpl->mHighWaterMark);
    // Bind
    pImpl->mServer->bind(endPoint);

    pImpl->mSecurityLevel = Authentication::SecurityLevel::WOODHOUSE;
    pImpl->mEndPoint = endPoint;
    pImpl->mBound = true;
}

/// Add a subscription
void Router::addMessageType(
    std::unique_ptr<UMPS::MessageFormats::IMessage> &message)
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    if (!isBound())
    {
        throw std::runtime_error("Router not yet bound to socket");
    }
    auto messageType = message->getMessageType();
    if (messageType.empty())
    {
        throw std::invalid_argument("Message type is empty");
    }
    if (!pImpl->mMessageFormats.contains(messageType))
    {
        pImpl->mLogger->debug("Adding subscription: " + messageType);
        pImpl->mMessageFormats.add(message);
    }
    else
    {
        pImpl->mLogger->debug(messageType + " already exists");
    }
    auto idx = pImpl->mSubscriptions.find(messageType);
    if (idx == pImpl->mSubscriptions.end())
    {
        pImpl->mLogger->debug("Adding subscription: " + messageType);
        pImpl->mSubscriptions.insert(std::pair(messageType,
                                               std::move(message)));
    }
    else
    {
        pImpl->mLogger->debug("Overwriting subscription: " + messageType);
        idx->second = std::move(message);
    }
}
*/

/// Stop
void Router::stop()
{
    pImpl->stop();
}

/// Starts the service
void Router::start()
{
/*
    if (!isBound())
    {
        throw std::runtime_error("Router not yet bound to a socket");
    }
    
    if (!haveCallback())
    {
        throw std::runtime_error("Router does not have a callback");
    }
*/
    if (!isInitialized())
    {
         throw std::runtime_error("Router not initialized");
    }
    stop(); // Make sure service is stopped
    // Poll setup
    constexpr size_t nPollItems = 1;
    zmq::pollitem_t items[] =
    {
        {pImpl->mServer->handle(), 0, ZMQ_POLLIN, 0}
    };
    pImpl->start();
    auto logLevel = pImpl->mLogger->getLevel();
    while (isRunning()) 
    {
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS);
        if (items[0].revents & ZMQ_POLLIN)
        {
            // Get the next message
            zmq::multipart_t messagesReceived(*pImpl->mServer);
            if (messagesReceived.empty()){continue;}
            if (logLevel >= UMPS::Logging::Level::DEBUG)
            {
                pImpl->mLogger->debug("Message received!");
            }
/*
std::cout << messagesReceived.size() << std::endl;
std::cout << messagesReceived.at(0).to_string() << std::endl;
std::cout << messagesReceived.at(1).to_string() << std::endl;
std::cout << messagesReceived.at(2).to_string() << std::endl;
std::cout << messagesReceived.at(3).to_string() << std::endl;
*/
#ifndef NDEBUG
            assert(messagesReceived.size() == 4);
#else
            if (messagesReceived.size() != 4)
            {
                pImpl->mLogger->error("Only 2-part messages handled");
                continue; 
            }
#endif
            std::string messageType = messagesReceived.at(2).to_string();
/*
            if (!pImpl->mMessageFormats.contains(messageType))
            {
                auto errorMsg = "Unhandled message type: " + messageType;
                pImpl->mLogger->error(errorMsg);
                continue;
            }
            auto index = pImpl->mSubscriptions.find(messageType);
            if (index == pImpl->mSubscriptions.end())
            {
                auto errorMsg = "Unhandled message type: " + messageType;
                pImpl->mLogger->error(errorMsg);
                continue;
            }
*/
            auto messageContents = reinterpret_cast<const void *>
                                   (messagesReceived.at(3).data());
            auto messageSize = messagesReceived.at(3).size();
            auto response = pImpl->mCallback(messageType,
                                             messageContents, messageSize);
            // Send the response back
            auto responseMessageType = response->getMessageType();
            auto responseMessage = response->toMessage(); 
            if (responseMessage.empty())
            {
                pImpl->mLogger->warn("Router received empty message");
            }
            if (logLevel >= UMPS::Logging::Level::DEBUG)
            {
                pImpl->mLogger->debug("Replying...");
            }
            zmq::const_buffer zmqHdr1{messagesReceived.at(0).data(),
                                      messagesReceived.at(0).size()};
            pImpl->mServer->send(zmqHdr1, zmq::send_flags::sndmore);
            zmq::const_buffer zmqHdr2{messagesReceived.at(1).data(),
                                      messagesReceived.at(1).size()};
            pImpl->mServer->send(zmqHdr2, zmq::send_flags::sndmore);
            zmq::const_buffer header{responseMessageType.data(),
                                     responseMessageType.size()};
            pImpl->mServer->send(header, zmq::send_flags::sndmore);
            zmq::const_buffer responseBuffer{responseMessage.data(),
                                             responseMessage.size()};
            pImpl->mServer->send(responseBuffer);
        }
    }
    pImpl->mLogger->debug("Service loop finished");
}

/// Is the router running?
bool Router::isRunning() const noexcept
{
    return pImpl->isRunning();
}

/// Security level
UMPS::Messaging::Authentication::SecurityLevel
    Router::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

void Router::operator()()
{
    start();
}

/// Access address
std::string Router::getConnectionString() const
{
    if (!isInitialized()){throw std::runtime_error("Router not initialized");}
    return pImpl->mEndPoint;
}
