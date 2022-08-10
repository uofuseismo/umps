#include <iostream>
#include <string>
#include <zmq.hpp>
#include "umps/messaging/xPublisherXSubscriber/publisher.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Publisher::PublisherImpl
{
public:
    /*
    PublisherImpl(std::shared_ptr<zmq::context_t> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger, int)
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
        mPublisher = std::make_unique<zmq::socket_t> (*mContext,
                                                      zmq::socket_type::pub);
    }
    */
    /// C'tor
    PublisherImpl(std::shared_ptr<UMPS::Messaging::Context> context,
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
        mPublisher = std::make_unique<zmq::socket_t> (*contextPtr,
                                                      zmq::socket_type::pub);
    }
    void disconnect()
    {
        if (mConnected)
        {
            mPublisher->disconnect(mAddress);
            mSocketDetails.clear();
            mConnected = false;
        }
    }
    void updateSocketDetails()
    {
        mSocketDetails.setAddress(mAddress);
        mSocketDetails.setSecurityLevel(mSecurityLevel);
        mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::Bind);
    }
//private:
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mPublisher{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    PublisherOptions mOptions;
    UCI::SocketDetails::XPublisher mSocketDetails;
    std::string mAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::Grasslands;
    bool mConnected = false;
    bool mInitialized = false;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> (nullptr, nullptr))
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (nullptr, logger))
{
}

/// C'tor
/*
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<PublisherImpl> (context, nullptr, 0))
{
}
*/

Publisher::Publisher(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<PublisherImpl> (context, nullptr))
{
}

/// C'tor
/*
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context,
                     std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (context, logger, 0))
{
}
*/

Publisher::Publisher(std::shared_ptr<UMPS::Messaging::Context> &context,
                     std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (context, logger)) 
{
}

/// Destructor
Publisher::~Publisher() = default;

/// Initialize
void Publisher::initialize(const PublisherOptions &options)
{
    // Check and copy options
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set on options");
    }
    pImpl->mOptions = options; 
    pImpl->mInitialized = false;
    // Disconnect from old connections
    pImpl->disconnect();
    // Create zap options
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    zapOptions.setSocketOptions(&*pImpl->mPublisher);
    // Set other options
    auto timeOut = static_cast<int> (options.getTimeOut().count());
    auto hwm = pImpl->mOptions.getHighWaterMark();
    if (hwm > 0){pImpl->mPublisher->set(zmq::sockopt::sndhwm, hwm);}
    if (timeOut >= 0)
    {
        pImpl->mPublisher->set(zmq::sockopt::sndtimeo, timeOut);
    }
    // (Re)establish connection
    auto address = pImpl->mOptions.getAddress();
    pImpl->mPublisher->connect(address);
    // Resolve the end point
    pImpl->mAddress = address;
    if (address.find("tcp") != std::string::npos ||
        address.find("ipc") != std::string::npos)
    {
        pImpl->mAddress = pImpl->mPublisher->get(zmq::sockopt::last_endpoint);
    }
    pImpl->mConnected = true;
    // Copy some last details
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    pImpl->updateSocketDetails();
    pImpl->mInitialized = true;
}

/// Initialized?
bool Publisher::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Send a message
void Publisher::send(const MessageFormats::IMessage &message)
{
    if (!isInitialized())
    {
        throw std::runtime_error("Publisher not initialized");
    }
    auto messageType = message.getMessageType();
    if (messageType.empty())
    {
        pImpl->mLogger->debug("Message type is empty");
    }
    auto messageContents = message.toMessage();
    if (messageContents.empty())
    {
        pImpl->mLogger->debug("Message contents are empty");
    }
    //pImpl->mLogger->debug("Sending message of type: " + messageType);
    zmq::const_buffer header{messageType.data(), messageType.size()};
    pImpl->mPublisher->send(header, zmq::send_flags::sndmore);
    zmq::const_buffer buffer{messageContents.data(), messageContents.size()};
    pImpl->mPublisher->send(buffer);
}

/// Security level
UAuth::SecurityLevel Publisher::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Disconnect
void Publisher::disconnect()
{
    pImpl->disconnect();
    pImpl->mInitialized = false;
} 

/// Endpoint
std::string Publisher::getEndPoint() const
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    return pImpl->mAddress;
}

/// Socket details
UCI::SocketDetails::XPublisher Publisher::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Publisher not initialized");
    }
    return pImpl->mSocketDetails;
}
