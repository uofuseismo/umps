#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <unistd.h>
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/logging/log.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;

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
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
            mSubscriber = std::make_unique<zmq::socket_t> (*mContext,
                                                         zmq::socket_type::sub);
        }
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
    /// C'tor
    SubscriberImpl(std::shared_ptr<zmq::context_t> &context,
                   std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(context),
        mSubscriber(std::make_unique<zmq::socket_t> (*mContext,
                                                     zmq::socket_type::sub)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
            mSubscriber = std::make_unique<zmq::socket_t> (*mContext,
                                                         zmq::socket_type::sub);
        }
    }
    /// Disconnect
    void disconnect()
    {
        if (mConnected)
        {
            mSubscriber->disconnect(mAddress);
            mConnected = false; 
        }
    }
 
    UMPS::MessageFormats::Messages mMessageTypes;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mSubscriber;
    ///std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    SubscriberOptions mOptions;
    std::string mAddress;
    Authentication::SecurityLevel mSecurityLevel
        = Authentication::SecurityLevel::GRASSLANDS;
    bool mMadeContext = true;
    bool mInitialized = false;
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

/*
Subscriber::Subscriber(std::shared_ptr<zmq::context_t> &context,
                       std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (context, logger))
{
}
*/

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

/// Initialize
void Subscriber::initialize(const SubscriberOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set on options");
    }
    if (!options.haveMessageTypes())
    {
        throw std::invalid_argument("Message types not set on options");
    }
    pImpl->mOptions = options;
    pImpl->mInitialized = false;
    // Disconnnect from old connections
    pImpl->disconnect(); 
    // Create zap options
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    zapOptions.setSocketOptions(&*pImpl->mSubscriber);
    // Set other options
    auto timeOut = static_cast<int> (options.getTimeOut().count());
    auto hwm = pImpl->mOptions.getHighWaterMark();
    if (hwm > 0){pImpl->mSubscriber->set(zmq::sockopt::rcvhwm, hwm);}
    if (timeOut >= 0)
    {
        pImpl->mSubscriber->set(zmq::sockopt::rcvtimeo, timeOut);
    }
    // (Re)establish connections
    auto address = options.getAddress();
    try
    {
        pImpl->mSubscriber->connect(address);
    }
    catch (const std::exception &e)
    {
        auto errmsg = "Subscriber failed to connect with error: "
                    + std::string(e.what());
        pImpl->mLogger->error(errmsg);
        throw std::runtime_error(errmsg);
    }
    // Resolve the end point
    pImpl->mAddress = address;
    if (address.find("tcp") != std::string::npos ||
        address.find("ipc") != std::string::npos)
    {
        pImpl->mAddress = pImpl->mSubscriber->get(zmq::sockopt::last_endpoint);
    }
    pImpl->mConnected = true;
    // Add the subscriptions
    pImpl->mMessageTypes = pImpl->mOptions.getMessageTypes();
    auto messageTypeMap = pImpl->mMessageTypes.get();
    for (const auto &messageType : messageTypeMap)
    {
        pImpl->mSubscriber->set(zmq::sockopt::subscribe, messageType.first);
    }
    // Set some final details
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    pImpl->mInitialized = true;
}

/// Initialized?
bool Subscriber::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Endpoint
std::string Subscriber::getEndPoint() const
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    return pImpl->mAddress;
}

/*
/// Connected?
bool Subscriber::isConnected() const noexcept
{
    return !pImpl->mEndPoints.empty();
}

/// Have subscriptions?
bool Subscriber::haveSubscriptions() const noexcept
{
    return !pImpl->mMessageTypes.empty();
}

/// Connect to an address if not already done so
void Subscriber::connect(const std::string &endPoint)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
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
    pImpl->mSecurityLevel = Authentication::SecurityLevel::GRASSLANDS;
}

/// Strawhouse
void Subscriber::connect(
    const std::string &endPoint,
    const bool isAuthenticationServer,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setStrawhouse(pImpl->mSubscriber.get(), isAuthenticationServer, domain);
    pImpl->mSubscriber->connect(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STRAWHOUSE;
}

/// Woodhouse
void Subscriber::connect(
    const std::string &endPoint,
    const Authentication::Certificate::UserNameAndPassword &credentials,
    const bool isAuthenticationServer,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setWoodhouse(pImpl->mSubscriber.get(), credentials,
                 isAuthenticationServer, domain);
    pImpl->mSubscriber->connect(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::WOODHOUSE;
}

/// Connect subscriber as a CURVE server
void Subscriber::connect(
    const std::string &endPoint,
    const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {   
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setStonehouseServer(pImpl->mSubscriber.get(), serverKeys, domain);
    pImpl->mSubscriber->connect(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
}

/// Connect subscriber as a CURVE client 
void Subscriber::connect(
    const std::string &endPoint,
    const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
    const UMPS::Messaging::Authentication::Certificate::Keys &clientKeys,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setStonehouseClient(pImpl->mSubscriber.get(),
                        serverKeys, clientKeys, domain);
    pImpl->mSubscriber->connect(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
}
*/

// 
/*
void Subscriber::connect(
    const std::string &endPoint,
    const Authentication::Certificate::Keys &serverKeys,
    const Authentication::Certificate::Keys &clientKeys, 
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    if (!serverKeys.havePublicKey())
    {   
        throw std::invalid_argument("Server public key not set");
    }   
    if (pImpl->mEndPoints.contains(endPoint))
    {   
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }   
    pImpl->mSubscriber->set(zmq::sockopt::zap_domain, domain);
    pImpl->mSubscriber->set(zmq::sockopt::curve_server, 0); 
    auto serverKey = serverKeys.getPublicTextKey();
    pImpl->mSubscriber->set(zmq::sockopt::curve_publickey, serverKey.data());
}
*/

/// Disconnect from endpoint
void Subscriber::disconnect()
{
    pImpl->disconnect();
    pImpl->mInitialized = false;
}

/*
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
    if (pImpl->mMessageTypes.contains(messageType))
    {
        pImpl->mLogger->debug("Already subscribed to message type: "
                            + messageType);
        return;
    }
    pImpl->mMessageTypes.add(message);
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
*/

/// Receive messages
std::unique_ptr<UMPS::MessageFormats::IMessage> Subscriber::receive() const
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    // Receive all parts of the message
    zmq::multipart_t messagesReceived(*pImpl->mSubscriber);
    if (messagesReceived.empty()){return nullptr;}
#ifndef NDEBUG
    assert(static_cast<int> (messagesReceived.size()) == 2);
#else
    if (static_cast<int> (messagesReceived.size()) != 2)
    {
        pImpl->mLogger->error("Only 2-part messages handled");
        throw std::runtime_error("Only 2-part messages handled");
    }
#endif
    std::string messageType = messagesReceived.at(0).to_string();
    if (!pImpl->mMessageTypes.contains(messageType))
    {
        auto errorMsg = "Unhandled message type: " + messageType;
        pImpl->mLogger->error(errorMsg); 
        throw std::runtime_error(errorMsg);
    }
    const auto payload = static_cast<char *> (messagesReceived.at(1).data());
    auto messageLength = messagesReceived.at(1).size();
    auto result = pImpl->mMessageTypes.get(messageType);
    try
    {
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

/// Security level
UMPS::Messaging::Authentication::SecurityLevel
    Subscriber::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
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
