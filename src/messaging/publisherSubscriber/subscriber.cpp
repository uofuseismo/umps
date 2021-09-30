#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <unistd.h>
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/logging/log.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;

/*
class Subscriber::SubscriberImpl
{
public:
    /// C'tor
    SubscriberImpl() :
//        mContext(std::make_unique<zmq::context_t> (0)),
//        mSubscriber(std::make_unique<zmq::socket_t> (&mContext,
//                                                     zmq::socket_type::sub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
        mContext = std::make_shared<void *> (zmq_ctx_new());
        mSubscriber = zmq_socket(*mContext, ZMQ_SUB);
    }
    /// C'tor
    SubscriberImpl(std::shared_ptr<void *> &context) :
        mContext(context),
        //mSubscriber(*context, zmq::socket_type::sub),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
        mMadeContext = false;
        if (mContext == nullptr)
        {
            mLogger->error("Context is NULL - creating one");
            mContext = std::make_shared<void *> (zmq_ctx_new());
            mMadeContext = true;
        }
        mSubscriber = zmq_socket(*mContext, ZMQ_SUB);
    }
    /// C'tor
    SubscriberImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
//        mContext(std::make_unique<zmq::context_t> (0)),
//        mSubscriber(std::make_unique<zmq::socket_t> (&mContext,
//                                                     zmq::socket_type::sub)),
        mLogger(logger)
    {
        mContext = std::make_shared<void *> (zmq_ctx_new());
        mSubscriber = zmq_socket(*mContext, ZMQ_SUB);
    }
    /// Destructor
    ~SubscriberImpl()
    {
        if (mSubscriber){zmq_close(mSubscriber);}
        if (mContext && mMadeContext){zmq_ctx_destroy(*mContext);}
        mEndPoints.clear();
        mSubscriber = nullptr;
        mContext = nullptr;
        mLogger = nullptr;
        mMadeContext = true;
        mConnected = false;
    }
    const int nContexts = 1;
//    zmq::context_t mContext{nContexts};//;//{0};
    //zmq::socket_t mSubscriber{mContext, zmq::socket_type::sub};
//    std::unique_ptr<zmq::socket_t> mSubscriber;//(*mContext, zmq::socket_type::pub);
    std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
        mSubscriptions;
    std::shared_ptr<void *> mContext = nullptr;
    void *mSubscriber = nullptr;
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    bool mMadeContext = true;
    bool mConnected = false;
};
*/

class Subscriber::SubscriberImpl
{
public:
    /// C'tor
    SubscriberImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mSubscriber(std::make_unique<zmq::socket_t> (*mContext,
                                                     zmq::socket_type::sub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    /// C'tor
    SubscriberImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mSubscriber(std::make_unique<zmq::socket_t> (*mContext,
                                                     zmq::socket_type::sub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
/*
        mMadeContext = false;
        if (mContext == nullptr)
        {
            mLogger->error("Context is NULL - creating one");
            mContext = std::make_shared<void *> (zmq_ctx_new());
            mMadeContext = true;
        }
        mSubscriber = zmq_socket(*mContext, ZMQ_SUB);
*/
    }
    /// C'tor
    SubscriberImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mSubscriber(std::make_unique<zmq::socket_t> (*mContext,
                                                     zmq::socket_type::sub)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
 
    std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
        mSubscriptions;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mSubscriber;
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    int mHighWaterMark = 4*1024;
    bool mMadeContext = true;
    bool mConnected = false;
};

/// C'tor
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> ())
{
}

Subscriber::Subscriber(std::shared_ptr<zmq::context_t> &context) :
   pImpl(std::make_unique<SubscriberImpl> (context))
{
}

/// C'tor
Subscriber::Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (logger))
{
}

/// Move c'tor
Subscriber::Subscriber(Subscriber &&subscriber) noexcept
{
    *this = std::move(subscriber);
}

/// Destructor
Subscriber::~Subscriber() = default;

/// Move assignment
Subscriber& Subscriber::operator=(Subscriber &&subscriber) noexcept
{
    if (&subscriber == this){return *this;}
    pImpl = std::move(subscriber.pImpl);
    return *this;
}

/// Connected?
bool Subscriber::isConnected() const noexcept
{
    return !pImpl->mEndPoints.empty();
}

/// Have subscriptions?
bool Subscriber::haveSubscriptions() const noexcept
{
    return !pImpl->mSubscriptions.empty();
}

/*
void Subscriber::addSubscription(
    std::unique_ptr<UMPS::MessageFormats::IMessage> &message)
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
    auto zmqError = zmq_setsockopt(pImpl->mSubscriber, ZMQ_SUBSCRIBE,
                                   messageType.c_str(), messageType.size());
    if (zmqError != 0)
    {
        auto zmqErrorMsg = zmq_strerror(zmqError);
        auto errorMsg = "Failed to subscribe to filter: " + messageType + "\n"
                      + "ZMQ failed with:\n" + zmqErrorMsg;
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
}
*/

/// Connect to an address if not already done so
void Subscriber::connect(const std::string &endPoint)
{
    auto idx = pImpl->mEndPoints.find(endPoint);
    if (idx == pImpl->mEndPoints.end())
    {
        pImpl->mLogger->debug("Attempting to connect to: " + endPoint);
    }
    else
    {
        if (idx->second)
        {
            pImpl->mLogger->info("Already connected to end point: " + endPoint);
            return;
        }
        else
        {
            pImpl->mLogger->info("attempting to reconnect to: " + endPoint);
        }
    }
    // Connect
    try
    {
        pImpl->mSubscriber->connect(endPoint);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Failed to connect to endpoint: " + endPoint
                      + "\nZeroMQ failed with: " + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    // Set the highwater mark - this makes the subscriber cache messages on
    // its end.  The idea is that if something is going to crash and burn 
    // because too many message are coming in then it should be the
    // subscriber and not the producer.
    try
    {
        pImpl->mSubscriber->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
    }
    catch (const std::exception &e)
    {
        auto errorMsg
            = "Failed to connect to set high water mark: ZMQ failed with:\n"
            + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
}

void Subscriber::connect(
    const std::string &endPoint,
    const UMPS::Messaging::Authentication::Certificate::Keys &keys)
{
    pImpl->mSubscriber->set(zmq::sockopt::zap_domain, "global");
    pImpl->mSubscriber->set(zmq::sockopt::plain_username, "user");
    pImpl->mSubscriber->set(zmq::sockopt::plain_password, "password");
/*
    pImpl->mSubscriber->set(zmq::sockopt::curve_server, 0);
    //pImpl->mSubscriber->set(zmq::sockopt::zap_domain, "zeromq.zap.01");//"inproc://zeromq.zap.01");
    if (certificate.havePublicKey())
    {
        char serverPublicKey[41];
        auto publicKey = certificate.getPublicTextKey();
        std::copy(publicKey.begin(), publicKey.end(), serverPublicKey);
        pImpl->mSubscriber->set(zmq::sockopt::curve_serverkey, serverPublicKey);
    }
    if (certificate.havePublicKey())
    {
        char clientPublicKey[41];
        auto publicKey = certificate.getPublicTextKey();
        std::copy(publicKey.begin(), publicKey.end(), clientPublicKey);
        pImpl->mSubscriber->set(zmq::sockopt::curve_publickey, clientPublicKey);
        //auto publicKey = certificate.getPublicKey();
        //auto rc = zmq_setsockopt(pImpl->mSubscriber->handle(),
        //                         ZMQ_CURVE_PUBLICKEY, // Set on client
        //                         publicKey.data(), publicKey.size());
        //if (rc != 0)
        //{   
        //    throw std::runtime_error("Failed to set CURVE server public key");
        //}
    }
    if (certificate.havePrivateKey())
    {
        char clientPrivateKey[41];
        auto privateKey = certificate.getPrivateTextKey();
        std::copy(privateKey.begin(), privateKey.end(), clientPrivateKey);
        pImpl->mSubscriber->set(zmq::sockopt::curve_secretkey, clientPrivateKey);
        //auto privateKey = certificate.getPrivateKey();
        //auto rc = zmq_setsockopt(pImpl->mSubscriber->handle(),
        //                         ZMQ_CURVE_SECRETKEY,
        //                         privateKey.data(), privateKey.size());
        //if (rc != 0)
        //{   
        //    throw std::runtime_error("Failed to set CURVE server private key");
        //}
    }
*/
    pImpl->mSubscriber->connect(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
}

/// Disconnect from endpoint
void Subscriber::disconnect(const std::string &endpoint)
{
    auto idx = pImpl->mEndPoints.find(endpoint);
    if (idx == pImpl->mEndPoints.end())
    {
        pImpl->mLogger->debug("Endpoint: " + endpoint + " does not exist");
        return;
    }
    if (!idx->second)
    {
        pImpl->mLogger->debug("Not connected to: " + endpoint);
    }
    else
    {
        pImpl->mSubscriber->disconnect(endpoint);
    }
    pImpl->mEndPoints.erase(idx);
}

/// Add a subscription
void Subscriber::addSubscription(
    std::unique_ptr<UMPS::MessageFormats::IMessage> &message)
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
        pImpl->mSubscriber->set(zmq::sockopt::subscribe, messageType);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Failed to add filter: " + messageType
                      + "\nZeroMQ failed with: " + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
}

/// Receive messages
std::unique_ptr<UMPS::MessageFormats::IMessage> Subscriber::receive() const
{
    if (!isConnected()){throw std::runtime_error("Not connected");}
    if (!haveSubscriptions()){throw std::runtime_error("No subscriptions");}
    // Receive all parts of the message
    std::vector<zmq::message_t> messagesReceived;
    zmq::recv_result_t receivedResult =
        zmq::recv_multipart(*pImpl->mSubscriber,
                            std::back_inserter(messagesReceived));
//  assert(receivedResult && "recv failed");
#ifndef NDEBUG
    assert(*receivedResult == 2);
#else
    if (*receivedResult != 2)
    {
        pImpl->mLogger->error("Only 2-part messages handled");
        throw std::runtime_error("Only 2-part messages handled");
    }
#endif
    std::string messageType = messagesReceived.at(0).to_string();
    auto index = pImpl->mSubscriptions.find(messageType);
    if (index == pImpl->mSubscriptions.end())
    {
        auto errorMsg = "Unhandled message type: " + messageType;
        pImpl->mLogger->error(errorMsg); 
        throw std::runtime_error(errorMsg);
    }
    //const auto payload = static_cast<uint8_t *> (messagesReceived.at(1).data());
    const auto payload = static_cast<char *> (messagesReceived.at(1).data());
    auto messageLength = messagesReceived.at(1).size();
    auto result = index->second->createInstance();
    try
    {
        //result->fromCBOR(payload, messageLength);
        result->fromMessage(payload, messageLength);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Failed to unpack message of type: " + messageType;
        pImpl->mLogger->error(errorMsg);
        throw;
    }
    return result;
}

/*
/// Connect to an address if not already done so
void Subscriber::connect(const std::string &endPoint)
{
    auto idx = pImpl->mEndPoints.find(endPoint);
    if (idx == pImpl->mEndPoints.end())
    {
        pImpl->mLogger->debug("Attempting to connect to: " + endPoint);
    }
    else
    {
        if (idx->second)
        {
            pImpl->mLogger->info("Already connected to end point: " + endPoint);
            return;
        }
        else
        {
            pImpl->mLogger->info("attempting to reconnect to: " + endPoint);
        }
    }
    // Set the high water mark
    int highWaterMark = 5000;
    auto zmqError = zmq_setsockopt(pImpl->mSubscriber, ZMQ_RCVHWM,
                                   &highWaterMark, sizeof(highWaterMark));
    if (zmqError != 0)
    {
        auto zmqErrorMsg = zmq_strerror(zmqError);
        auto errorMsg = "Failed to set high water mark.\nZMQ failed with "
                      + std::string(zmqErrorMsg);
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    // Finally attempt to bind to address
    //pImpl->mSubscriber.connect(endPoint);
    //pImpl->mSubscriber.set(zmq::sockopt::subscribe, "Pick");
    // Finally attempt to bind to address
    zmqError = zmq_connect(pImpl->mSubscriber, endPoint.c_str());
    if (zmqError != 0)
    {
        auto zmqErrorMsg = zmq_strerror(zmqError);
        auto errorMsg = "Failed to connect to: " + endPoint + "\n"
                      + "ZMQ failed with:\n" + zmqErrorMsg;
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg); 
    }
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
//const std::string filter= "";
//zmqError = zmq_setsockopt(pImpl->mSubscriber, ZMQ_SUBSCRIBE,
//                          filter.c_str(), filter.size());
//if (zmqError != 0)
//{
//    auto zmqErrorMsg = zmq_strerror(zmqError);
//    auto errorMsg = "Failed to subscribe to filter: " + filter + "\n"
//                  + "ZMQ failed with:\n" + zmqErrorMsg;
//    pImpl->mLogger->error(errorMsg);
//    throw std::runtime_error(errorMsg);
//}
}
*/

/*
std::unique_ptr<UMPS::MessageFormats::IMessage> Subscriber::receive() const
{
    if (!isConnected()){throw std::runtime_error("Not connected");}
    if (!haveSubscriptions()){throw std::runtime_error("No subscriptions");}
    std::unique_ptr<UMPS::MessageFormats::IMessage> result = nullptr;
char mwork[64];
std::cout << "waitin" << std::endl;
int size = zmq_recv(pImpl->mSubscriber, mwork, 64, 0);//ZMQ_NOBLOCK);
std::cout << size << std::endl;
return result;
int nTries = 0;
//    while (true)
    int64_t more = 0;
    size_t moreSize = sizeof(more);
    do
    {
std::cout << "in here" << std::endl;
  char messageType[64];
        int size = zmq_recv(pImpl->mSubscriber, messageType, 64, ZMQ_NOBLOCK);
std::cout << size << std::endl;
        if (size == -1){std::cerr << "ahh" << std::endl;} 
        // Create an empty message
        zmq_msg_t messagePart;
        auto zmqError = zmq_msg_init(&messagePart);
        if (zmqError != 0)
        {
            auto zmqErrorMsg = zmq_strerror(zmqError);
            auto errorMsg = std::string("Failed to initialize message.\n")
                          + "ZMQ failed with: " + zmqErrorMsg;
            pImpl->mLogger->error(errorMsg);
            std::cerr << zmqErrorMsg << std::endl;
            break;
        }
        // Block until a message is available to be received
std::cout << "blocking" << std::endl;
        zmqError = zmq_msg_recv(&messagePart, pImpl->mSubscriber, 0);
std::cout << "part 1" << std::endl;
        if (zmqError != 0)
        {
            auto zmqErrorMsg = zmq_strerror(zmqError);
            std::cerr << zmqErrorMsg << std::endl;
            break;
        }
        // Determine if more message parts are to follow
        zmqError = zmq_getsockopt(pImpl->mSubscriber, ZMQ_RCVMORE,
                                  &more, &moreSize); 
        if (zmqError != 0)
        {
            std::cerr << zmqError << std::endl;
            break;
        }
        zmq_msg_close(&messagePart);
        //if (!zmq_msg_more(&message)){break;}
    } while(more);
//std::cout << nTries << std::endl;
//        zmq::message_t message;
//        auto res = pImpl->mSubscriber.recv(message, zmq::recv_flags::none);
//       if (!message.more()){std::cout << "sheep" << std::endl;}
//std::cout << "so sleepy" << std::endl;
//        sleep(1);
//std::cout << "im awake" << std::endl;
//        nTries = nTries + 1;
//        if (nTries == 10){break;}

//        std::vector<zmq::message_t> receivedMessages;
//        zmq::recv_result_t message =
//            zmq::recv_multipart(pImpl->mSubscriber,
//                                std::back_inserter(receivedMessages));
//        if (*message == 2){break;}
//break;
//    }
std::cout << "didit " << std::endl;
//    while (true)
//    {
//        zmq_msg_t message;
//        zmq_msg_init(&message);
//        zmq_msg_recv(&message, pImpl->mSubscriber, 0);
//        auto pick = std::make_unique<UMPS::MessageFormats::Pick> ();
//        zmq_msg_close(&message);
//        if (!zmq_msg_more(&message)){break;}
//    }
    return result;
}
*/

/*
/// Send a message
void Publisher::send(const size_t nBytes, const char *message)
{
    if (nBytes == 0)
    {
        pImpl->mLogger->info("Message is empty - will not publish");
        return;
    }
    if (message == nullptr)
    {
        pImpl->mLogger->error("Message is NULL");
        throw std::invalid_argument("Message is NULL");
    }
    if (pImpl->mEndPoints.empty())
    {
        pImpl->mLogger->error("No end points - run bind first");
        throw std::runtime_error("No end points - run bind first");
    }
    auto zmqError = zmq_send(pImpl->mSubscriber, message, nBytes, 0);
    if (zmqError != 0)
    {
        auto zmqErrorMsg = std::string(zmq_strerror(zmqError));
        auto errorMsg = "Failed to send message with error: " + zmqErrorMsg;
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
}

/// Send a message
void Publisher::send(const std::string &message)
{
    send(message.size(), message.c_str()); 
}

void Publisher::send(const std::vector<uint8_t> &message)
{
    auto nBytes = message.size();
    if (nBytes == 0)
    {
        pImpl->mLogger->info("Message is empty - will not publish");
        return;
    }
    zmq::message_t zmqMessage(nBytes);
    std::memcpy(zmqMessage.data(), message.data(), nBytes);
    pImpl->mSubscriber.send(zmqMessage);
}
*/
