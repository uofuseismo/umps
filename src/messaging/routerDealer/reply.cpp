#include <mutex>
#include <string>
#include <functional>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/logging/standardOut.hpp"
#include "private/messaging/requestReplySocket.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Reply::ReplyImpl : public ::ReplySocket
{
public:
    /// C'tor
    ReplyImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
              const std::shared_ptr<UMPS::Logging::ILog> &logger) :
        ::ReplySocket(context, logger) 
    {
/*
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
        // Now make the socket
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mServer = std::make_unique<zmq::socket_t> (*contextPtr,
                                                   zmq::socket_type::rep);
*/
    }
/*
    /// Indicate that the service is started/running
    void setRunning(const bool running = true)
    {
        std::scoped_lock lock(mMutex);
        mRunning = running;
    }
    /// Determines if the service was started
    [[nodiscard]] bool isRunning() const noexcept
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
            mServer->disconnect(mAddress);
            mAddress.clear();
            mSocketDetails.clear();
            mConnected = false;
        }
    }
    /// Update socket details
    void updateSocketDetails()
    {   
        mSocketDetails.setAddress(mAddress);
        mSocketDetails.setSecurityLevel(mSecurityLevel);
        mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::Bind);
    }
*/
///private:
    ReplyOptions mOptions;
};

/// C'tor
Reply::Reply() :
    pImpl(std::make_unique<ReplyImpl> (nullptr, nullptr))
{
}

Reply::Reply(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplyImpl> (nullptr, logger))
{
}

Reply::Reply(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ReplyImpl> (context, nullptr)) 
{
}

Reply::Reply(std::shared_ptr<UMPS::Messaging::Context> &context,
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
    if (!options.haveAddress())
    {
        throw std::invalid_argument("End point not set");
    }
    // Hangup
    pImpl->mOptions.clear();
    pImpl->disconnect();
    // Connect
    pImpl->connect(options);
    pImpl->mOptions = options;
/*    
    auto socketOptions = pImpl->convertSocketOptions(options);
    pImpl->connect(socketOptions);
    socketOptions.setAddress(options.getAddress());
    socketOptions.setCallback(options.getCallback());
    socketOptions.setZAPOptions(options.getZAPOptions());
    socketOptions.setSendHighWaterMark(options.getHighWaterMark());
    socketOptions.setReceiveHighWaterMark(options.getHighWaterMark());
    if (pImpl->mOptions.haveRoutingIdentifier())
    socketOptions.setRoutingIdentifier();
    auto address = pImpl->mOptions.getAddress();
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    auto receiveHighWaterMark = pImpl->mOptions.getHighWaterMark();
    auto sendHighWaterMark = pImpl->mOptions.getHighWaterMark();
    // Set the ZAP options
    zapOptions.setSocketOptions(&*pImpl->mServer);
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    // Set the high water mark
    pImpl->mServer->set(zmq::sockopt::rcvhwm, receiveHighWaterMark);
    pImpl->mServer->set(zmq::sockopt::sndhwm, sendHighWaterMark);
    if (pImpl->mOptions.haveRoutingIdentifier())
    {
        pImpl->mServer->set(zmq::sockopt::routing_id,
                            pImpl->mOptions.getRoutingIdentifier());
    }
    // Set the callback 
    pImpl->mCallback = pImpl->mOptions.getCallback();
    pImpl->mHaveCallback = true;
    // Connect 
    pImpl->mLogger->debug("Reply attempting to connect to: " + address);
    pImpl->mServer->connect(address);
    pImpl->mLogger->debug("Reply connected to: " + address);
    // Resolve end point
    pImpl->mAddress = address;
    if (address.find("tcp") != std::string::npos ||
        address.find("ipc") != std::string::npos)
    {   
        pImpl->mAddress = pImpl->mServer->get(zmq::sockopt::last_endpoint);
    }
    pImpl->mConnected = true; 
//    pImpl->updateSocketDetails();
    pImpl->mInitialized = true;
    pImpl->mLogger->debug("Reply initialized!");
*/
}

/// Initialized?
bool Reply::isInitialized() const noexcept
{
    return pImpl->isConnected();
}

/// Starts the service
void Reply::start()
{
    if (!isInitialized())
    {
         throw std::runtime_error("Reply not initialized");
    }
    pImpl->start();
/*
    stop(); // Make sure service is stopped
    // Poll setup
    constexpr size_t nPollItems = 1;
    zmq::pollitem_t items[] =
    {
        {pImpl->mServer->handle(), 0, ZMQ_POLLIN, 0}
    };  
    pImpl->setRunning(true);
    while (isRunning()) 
    {   
        auto logLevel = pImpl->mLogger->getLevel();
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS);
        if (items[0].revents & ZMQ_POLLIN)
        {
            // Get the next message
            zmq::multipart_t messagesReceived(*pImpl->mServer);
            //if (messagesReceived.empty()){continue;}
            if (logLevel >= UMPS::Logging::Level::Debug)
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
            std::string responseMessageType;
            std::string responseMessage;
            try
            {
                auto response = pImpl->mCallback(messageType,
                                                 messageContents, messageSize);
                if (response != nullptr)
                {
                    // Serialize the response
                    responseMessageType = response->getMessageType();
                    responseMessage = response->toMessage();
                    if (responseMessage.empty())
                    {
                        pImpl->mLogger->warn("Reply received empty message");
                    }
                }
                else
                {
                    pImpl->mLogger->warn("Response is NULL check calllback");
                }
            }
            catch (const std::exception &e)
            {
                pImpl->mLogger->error("Error in callback/serialization: "
                                   + std::string(e.what()));
            }
            if (logLevel >= UMPS::Logging::Level::Debug)
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
*/
}

/// Is the router running?
bool Reply::isRunning() const noexcept
{
    return pImpl->isRunning();
}

/// Stop
void Reply::stop()
{
    pImpl->setRunning(false);
}

/// Connection details
UCI::SocketDetails::Reply Reply::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Reply not initialized");
    }
    return pImpl->getSocketDetails();
}
