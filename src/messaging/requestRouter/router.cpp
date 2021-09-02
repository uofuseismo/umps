#include <map>
#include <string>
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
          (const URTS::MessageFormats::IMessage *)
    > mCallback;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{0};
    mutable std::mutex mMutex;
    bool mRunning = false; 
    bool mConnected = false;
    bool mInitialized = false;
};

/// C'tor
Router::Router() :
    pImpl(std::make_unique<RouterImpl> ())
{
}

/// Destructor
Router::~Router() = default;

void Router::initialize(const std::string &endPoint,
    const std::function<std::unique_ptr<URTS::MessageFormats::IMessage> (const URTS::MessageFormats::IMessage *)> &callback)
{
    pImpl->mServer->bind(endPoint);
    pImpl->mCallback = callback;
}

/// Connected?
bool Router::isConnected() const noexcept
{
    return pImpl->mConnected;
}

/// Add a subscription
void Router::addSubscription(
    std::unique_ptr<URTS::MessageFormats::IMessage> &message)
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    if (!isConnected())
    {
        throw std::runtime_error("Subscriber not yet connected");
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
    // Listen on this topic
    pImpl->mLogger->debug("Subscribing to: " + messageType);
    try
    {
        pImpl->mServer->set(zmq::sockopt::subscribe, messageType);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Failed to add filter: " + messageType
                      + "ZMQ failed with:\n" + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
}

/// Starts the service
void Router::start()
{
    if (!isConnected())
    {
        throw std::runtime_error("Subscriber not yet connected");
    }
    constexpr size_t nPollItems = 1;
    zmq::pollitem_t items[] =
    {
        {pImpl->mServer->handle(), 0, ZMQ_POLLIN, 0}
    };
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
#ifndef NDEBUG
            assert(*receivedResult == 2); 
#else
            if (*receivedResult != 2)
            {
                pImpl->mLogger->error("Only 2-part messages handled");
                returnDumby = true;
            }
#endif
            std::string messageType = messagesReceived.at(0).to_string();
            auto index = pImpl->mSubscriptions.find(messageType);
            if (index == pImpl->mSubscriptions.end())
            {
                auto errorMsg = "Unhandled message type: " + messageType;
                pImpl->mLogger->error(errorMsg);
                returnDumby = true;
            }
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
            // Send the response back
            auto cborResponseMessageType = response->getMessageType();
            auto cborResponseMessage = std::string(response->toCBOR());
            if (cborResponseMessage.empty())
            {
                pImpl->mLogger->debug("CBOR message is empty");
            }
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
