#include <iostream>
#include <mutex>
#include <string>
#include <functional>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UAuth = UMPS::Authentication;

class Reply::ReplyImpl
{
public:
    /// C'tor
    ReplyImpl(std::shared_ptr<zmq::context_t> context,
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
                                                   zmq::socket_type::rep);
    }
    /// Indicate that the service is started/running
    void start()
    {
        std::scoped_lock lock(mMutex);
        mRunning = true;
    }
    /// Indicate that the service is stopped/not running
    void stop()
    {
        std::scoped_lock lock(mMutex);
        mRunning = false;
    }
    /// Determines if the service was started
    bool isRunning() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mRunning;
    }
    // Disconnect
    void disconnect()
    {
       std::scoped_lock lock(mMutex);
       if (mConnected)
       {
           mServer->disconnect(mEndPoint);
           mEndPoint.clear();
           mConnected = false;
       }
    }

///private:
    mutable std::mutex mMutex;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mServer;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    std::function<
          std::unique_ptr<UMPS::MessageFormats::IMessage>
          (const std::string &messageType, const void *contents,
           const size_t length)
    > mCallback;
    ReplyOptions mOptions;
    std::string mEndPoint;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{10};
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
    bool mHaveCallback = false;
    bool mRunning = false;
    bool mConnected = false;
    bool mInitialized = false;
};

/// C'tor
Reply::Reply() :
    pImpl(std::make_unique<ReplyImpl> (nullptr, nullptr))
{
}

/// C'tor
Reply::Reply(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplyImpl> (nullptr, logger))
{
}

/// C'tor
Reply::Reply(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<ReplyImpl> (context, nullptr))
{
}

/// C'tor
Reply::Reply(std::shared_ptr<zmq::context_t> &context,
             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplyImpl> (context, logger))
{
}

/// Destructor
Reply::~Reply() = default;

/// Initialize
void Reply::initialize(const ReplyOptions &options)
{
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
    if (!options.haveEndPoint())
    {
        throw std::invalid_argument("End point not set");
    }
    // Hangup
    pImpl->mOptions.clear();
    pImpl->disconnect();
    pImpl->mOptions = options;
    // Get zap options
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
    // Connect 
    pImpl->mServer->connect(endPoint);
    // Resolve end point
    pImpl->mEndPoint = endPoint;
    if (endPoint.find("tcp") != std::string::npos ||
        endPoint.find("ipc") != std::string::npos)
    {   
        pImpl->mEndPoint = pImpl->mServer->get(zmq::sockopt::last_endpoint);
    }
    pImpl->mConnected = true; 
    pImpl->mInitialized = true;
}

/// Initialized?
bool Reply::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Starts the service
void Reply::start()
{
    if (!isInitialized())
    {
         throw std::runtime_error("Reply not initialized");
    }
    stop(); // Make sure service is stopped
    // Poll setup
    constexpr size_t nPollItems = 1;
    zmq::pollitem_t items[] =
    {
        {pImpl->mServer->handle(), 0, ZMQ_POLLIN, 0}
    };  
    pImpl->start();
    while (isRunning()) 
    {   
        auto logLevel = pImpl->mLogger->getLevel();
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS);
        if (items[0].revents & ZMQ_POLLIN)
        {
            // Get the next message
            zmq::multipart_t messagesReceived(*pImpl->mServer);
            if (messagesReceived.empty()){continue;}
            if (logLevel >= UMPS::Logging::Level::DEBUG)
            {
                pImpl->mLogger->debug("Reply received message!");
            }
#ifndef NDEBUG
            assert(messagesReceived.size() == 2);
#else
            if (messagesReceived.size() != 2)
            {
                pImpl->mLogger->error("Only 2-part messages handled");
                continue;
            }
#endif
            std::string messageType = messagesReceived.at(0).to_string();
            auto messageContents = reinterpret_cast<const void *>
                                   (messagesReceived.at(1).data());
            auto messageSize = messagesReceived.at(1).size();
            auto response = pImpl->mCallback(messageType,
                                             messageContents, messageSize);
            // Send the response back
            auto responseMessageType = response->getMessageType();
            auto responseMessage = response->toMessage();
            if (responseMessage.empty())
            {
                pImpl->mLogger->warn("Reply received empty message");
            }
            if (logLevel >= UMPS::Logging::Level::DEBUG)
            {
                pImpl->mLogger->debug("Replying...");
            }
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
bool Reply::isRunning() const noexcept
{
    return pImpl->isRunning();
}

/// Stop
void Reply::stop()
{
    pImpl->stop();
}