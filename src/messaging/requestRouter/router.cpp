#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RequestRouter;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Router::RouterImpl
{
public:
    /*
    /// C'tor
    RouterImpl(std::shared_ptr<zmq::context_t> context,
               std::shared_ptr<UMPS::Logging::ILog> logger, int )
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
    */
    /// C'tor
    RouterImpl(std::shared_ptr<UMPS::Messaging::Context> context,
               std::shared_ptr<UMPS::Logging::ILog> logger)
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
        // Now make the socket
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mServer = std::make_unique<zmq::socket_t> (*contextPtr,
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
           mServer->disconnect(mAddress);
           mAddress.clear();
           mBound = false;
       }
    }
    /// Update socket details
    void updateSocketDetails()
    {
        mSocketDetails.setAddress(mAddress);
        mSocketDetails.setSecurityLevel(mSecurityLevel);
        mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::CONNECT);
    }
//    std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
        //mSubscriptions;
//    UMPS::MessageFormats::Messages mMessageFormats;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mServer{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::function<
          std::unique_ptr<UMPS::MessageFormats::IMessage>
          (const std::string &messageType, const void *contents,
           const size_t length)
    > mCallback;
    RouterOptions mOptions;
    UCI::SocketDetails::Router mSocketDetails;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{10};
    mutable std::mutex mMutex;
    std::string mAddress;
    int mHighWaterMark = 100; 
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
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

/// C'tor
Router::Router(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RouterImpl> (nullptr, logger))
{
}

/// C'tor
/*
Router::Router(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<RouterImpl> (context, nullptr, 0))
{
}
*/

Router::Router(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RouterImpl> (context, nullptr))
{
}

/// C'tor
/*
Router::Router(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RouterImpl> (context, logger, 0))
{
}
*/

Router::Router(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RouterImpl> (context, logger)) 
{
}

/*
/// Move c'tor
Router::Router(Router &&router) noexcept
{
    *this = std::move(router);
}

/// Move assignment
Router& Router::operator=(Router &&router) noexcept
{
    if (&router == this){return *this;}
    pImpl = std::move(router.pImpl);
    return *this;
}    
*/

/// Destructor
Router::~Router() = default;

/// Initialized?
bool Router::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Initializes the router
void Router::initialize(const RouterOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
//    pImpl->mMessageFormats = options.getMessageFormats();
//    if (pImpl->mMessageFormats.empty())
//    {
//        pImpl->mLogger->warn("No message types set in options");
//    }

    pImpl->mOptions.clear();
    pImpl->unbind();
    pImpl->mOptions = options;
    //  
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    auto highWaterMark = pImpl->mOptions.getHighWaterMark();
    auto address = pImpl->mOptions.getAddress();
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
    pImpl->mServer->bind(address);
    // Resolve end point
    pImpl->mAddress = address;
    if (address.find("tcp") != std::string::npos ||
        address.find("ipc") != std::string::npos)
    {
        pImpl->mAddress = pImpl->mServer->get(zmq::sockopt::last_endpoint);
    }
    pImpl->updateSocketDetails();
    pImpl->mBound = true;
    pImpl->mInitialized = true;
}

/// Stop
void Router::stop()
{
    pImpl->stop();
}

/// Starts the service
void Router::start()
{
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
UAuth::SecurityLevel Router::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

void Router::operator()()
{
    start();
}

/// Access address
std::string Router::getAddress() const
{
    if (!isInitialized()){throw std::runtime_error("Router not initialized");}
    return pImpl->mAddress;
}

/// Connection information
UCI::SocketDetails::Router Router::getSocketDetails() const
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Router not initialized");
    }   
    return pImpl->mSocketDetails;
}
