#include <iostream>
#include <string>
#include <zmq.hpp>
#include "umps/messaging/xPublisherXSubscriber/publisher.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;

class Publisher::PublisherImpl
{
public:
    /// C'tor
    PublisherImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::xpub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()) 
    {
    }
    /// C'tor
    explicit PublisherImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::xpub)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }

    void disconnect()
    {
        if (mConnected)
        {
            mPublisher->disconnect(mAddress);
            mConnected = false;
        }
    }
//private:
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mPublisher;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    PublisherOptions mOptions;
    std::string mAddress;
    Authentication::SecurityLevel mSecurityLevel
        = Authentication::SecurityLevel::GRASSLANDS;
    bool mConnected = false;
    bool mInitialized = false;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> ())
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (logger))
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
    auto address = pImpl->mOptions.getAddress();
    auto hwm = pImpl->mOptions.getHighWaterMark();
    if (hwm > 0){pImpl->mPublisher->set(zmq::sockopt::sndhwm, hwm);}
    // (Re)establish connection
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
UMPS::Messaging::Authentication::SecurityLevel 
    Publisher::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Disconnect
void Publisher::disconnect()
{
    pImpl->disconnect();
    pImpl->mInitialized = false;
} 
