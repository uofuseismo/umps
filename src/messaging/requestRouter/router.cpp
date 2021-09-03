#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "urts/messaging/requestRouter/router.hpp"
#include "urts/messageFormats/message.hpp"
#include "urts/logging/stdout.hpp"

using namespace URTS::Messaging::RequestRouter;

class Router::RouterImpl
{
public:
    /// C'tor
    RouterImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mServer(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::router)),
        mLogger(std::make_shared<URTS::Logging::StdOut> ())
    {
    }
    /// C'tor with specified logger
    explicit RouterImpl(std::shared_ptr<URTS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mServer(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::router)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<URTS::Logging::StdOut> ();
        }
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

    std::map<std::string, std::unique_ptr<URTS::MessageFormats::IMessage>> 
        mSubscriptions;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mServer;
    std::shared_ptr<URTS::Logging::ILog> mLogger = nullptr;
    std::function<
          std::unique_ptr<URTS::MessageFormats::IMessage>
          (const std::string &messageType, const uint8_t *contents, const size_t length) //(const URTS::MessageFormats::IMessage *)
    > mCallback;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{10};
    mutable std::mutex mMutex;
    std::map<std::string, bool> mEndPoints;
    bool mRunning = false; 
    bool mConnected = false;
    bool mHaveCallback = false;
    bool mInitialized = false;
};

/// C'tor
Router::Router() :
    pImpl(std::make_unique<RouterImpl> ())
{
}

Router::Router(std::shared_ptr<URTS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RouterImpl> (logger))
{
}

/// Destructor
Router::~Router() = default;

//void Router::initialize(const std::string &endPoint,
void Router::setCallback(
    //const std::function<std::unique_ptr<URTS::MessageFormats::IMessage> (const URTS::MessageFormats::IMessage *)> &callback)
    const std::function<std::unique_ptr<URTS::MessageFormats::IMessage>
                        (const std::string &, const uint8_t *, size_t)> &callback)
{
 //   pImpl->mServer->bind(endPoint);
    pImpl->mCallback = callback;
    pImpl->mHaveCallback = true;
}

bool Router::haveCallback() const noexcept
{
    return pImpl->mHaveCallback;
}

/// Connected?
bool Router::isBound() const noexcept
{
    for (const auto &endpoint : pImpl->mEndPoints)
    {
        if (endpoint.second){return true;}
    }
    return false;
}

/// Bind to an address if not already done so
void Router::bind(const std::string &endPoint)
{
    auto idx = pImpl->mEndPoints.find(endPoint);
    if (idx == pImpl->mEndPoints.end())
    {
        pImpl->mLogger->debug("Attempting to bind to: " + endPoint);
    }
    else
    {
        if (idx->second)
        {
            pImpl->mLogger->info("Already bound to end point: " + endPoint);
            return;
        }
        else
        {
            pImpl->mLogger->info("Attempting to rebind to: " + endPoint);
        }
    }
    try
    {
        pImpl->mServer->bind(endPoint);
        pImpl->mServer->set(zmq::sockopt::rcvhwm, 100);// pImpl->mHighWaterMark);
        pImpl->mServer->set(zmq::sockopt::sndhwm, 100);//pImpl->mHighWaterMark);
    }
    catch (const std::exception &e) 
    {
        auto errorMsg = "Failed to connect to endpoint: " + endPoint
                      + "ZeroMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
}

/*
/// Add a subscription
void Router::addSubscription(
    std::unique_ptr<URTS::MessageFormats::IMessage> &message)
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
//    // Listen on this topic
//    pImpl->mLogger->debug("Subscribing to: " + messageType);
//    try
//    {
//        pImpl->mServer->set(zmq::sockopt::subscribe, messageType);
//    }
//    catch (const std::exception &e)
//    {
//        auto errorMsg = "Failed to add filter: " + messageType
//                      + "ZMQ failed with:\n" + std::string(e.what());
//        pImpl->mLogger->error(errorMsg);
//        throw std::runtime_error(errorMsg);
//    }
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
    if (!isBound())
    {
        throw std::runtime_error("Router not yet bound to a socket");
    }
    if (!haveCallback())
    {
        throw std::runtime_error("Router does not have a callback");
    }
    stop(); 
    constexpr size_t nPollItems = 1;
    zmq::pollitem_t items[] =
    {
        {pImpl->mServer->handle(), 0, ZMQ_POLLIN, 0}
    };
    pImpl->start();
    while (isRunning()) 
    {
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS);
        if (items[0].revents & ZMQ_POLLIN)
        {
            // Get the next message
            std::vector<zmq::message_t> messagesReceived;
            zmq::recv_result_t receivedResult =
                zmq::recv_multipart(*pImpl->mServer,
                                    std::back_inserter(messagesReceived),
                                    zmq::recv_flags::none);
            bool returnDumby = false;
            if (*receivedResult == 0){continue;}
/*
std::cout << *receivedResult << std::endl;
std::cout << messagesReceived.size() << std::endl;
std::cout << messagesReceived.at(0).to_string() << std::endl;
std::cout << messagesReceived.at(1).to_string() << std::endl;
std::cout << messagesReceived.at(2).to_string() << std::endl;
std::cout << messagesReceived.at(3).to_string() << std::endl;
*/
#ifndef NDEBUG
            assert(messagesReceived.size() == 4);
            //assert(*receivedResult == 4); 
#else
            if (messagesReceived.size() != 4)
            {
                pImpl->mLogger->error("Only 2-part messages handled");
                returnDumby = true;
            }
#endif
            std::string messageType = messagesReceived.at(2).to_string();
/*
            auto index = pImpl->mSubscriptions.find(messageType);
            if (index == pImpl->mSubscriptions.end())
            {
                auto errorMsg = "Unhandled message type: " + messageType;
                pImpl->mLogger->error(errorMsg);
                returnDumby = true;
            }
*/
            auto messageContents = reinterpret_cast<const uint8_t *>
                                   (messagesReceived.at(3).data());
            auto messageSize = messagesReceived.at(3).size();
            auto response = pImpl->mCallback(messageType,
                                             messageContents, messageSize);
/*
            const auto payload = static_cast<uint8_t *> (messagesReceived.at(1).data());
            auto messageLength = messagesReceived.at(1).size();
            auto message = index->second->createInstance();
            try
            {
                message->fromCBOR(payload, messageLength);
            }
            catch (const std::exception &e)
            {
               auto errorMsg = "Failed to unpack message of type: "
                             + messageType;
               pImpl->mLogger->error(errorMsg);
               returnDumby = true;
            }
            // Process the message with the provided callback
            auto response = pImpl->mCallback(&*message);
*/
            // Send the response back
            auto cborResponseMessageType = response->getMessageType();
            auto cborResponseMessage = std::string(response->toCBOR());
            if (cborResponseMessage.empty())
            {
                pImpl->mLogger->debug("CBOR message is empty");
            }
            pImpl->mLogger->debug("Replying...");
            zmq::const_buffer zmqHdr1{messagesReceived.at(0).data(),
                                      messagesReceived.at(0).size()};
            pImpl->mServer->send(zmqHdr1, zmq::send_flags::sndmore);
            zmq::const_buffer zmqHdr2{messagesReceived.at(1).data(),
                                      messagesReceived.at(1).size()};
            pImpl->mServer->send(zmqHdr2, zmq::send_flags::sndmore);
            zmq::const_buffer header{cborResponseMessageType.data(),
                                     cborResponseMessageType.size()};
            pImpl->mServer->send(header, zmq::send_flags::sndmore);
            zmq::const_buffer responseBuffer{cborResponseMessage.data(),
                                             cborResponseMessage.size()};
            pImpl->mServer->send(responseBuffer);
        }
    }
    pImpl->mLogger->debug("Service loop finished");
}

bool Router::isRunning() const noexcept
{
    return pImpl->isRunning();
}

void Router::operator()()
{
    start();
}
