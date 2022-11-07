#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/logging/standardOut.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Subscriber::SubscriberImpl
{
public:
    /// C'tor
    SubscriberImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
                   const std::shared_ptr<UMPS::Logging::ILog> &logger)
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
            mLogger = std::make_shared<UMPS::Logging::StandardOut> ();
        }
        else
        {
            mLogger = logger;
        }
        // Now make the socket
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mSubscriber = std::make_unique<zmq::socket_t> (*contextPtr,
                                                       zmq::socket_type::sub);
    }
    /// Disconnect
    void disconnect()
    {
        if (mConnected)
        {
            mSubscriber->disconnect(mAddress);
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
    UMPS::MessageFormats::Messages mMessageTypes;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mSubscriber{nullptr};
    ///std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    SubscriberOptions mOptions;
    UCI::SocketDetails::Subscriber mSocketDetails;
    std::string mAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::Grasslands;
    bool mInitialized = false;
    bool mConnected = false;
};

/// Constructors
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
{
}

Subscriber::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<SubscriberImpl> (context, nullptr)) 
{
}

Subscriber::Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, logger))
{
}

Subscriber::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context,
                       std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (context, logger)) 
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
    auto timeOut = static_cast<int> (options.getReceiveTimeOut().count());
    auto hwm = pImpl->mOptions.getReceiveHighWaterMark();
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
    pImpl->updateSocketDetails();
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

/// Disconnect from endpoint
void Subscriber::disconnect()
{
    pImpl->disconnect();
    pImpl->mInitialized = false;
}

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
UAuth::SecurityLevel Subscriber::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Socket details
UCI::SocketDetails::Subscriber Subscriber::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Subscriber not initialized");
    }
    return pImpl->mSocketDetails;
}
