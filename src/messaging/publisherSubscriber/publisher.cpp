#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/publisherSubscriber/publisherOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/enums.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Publisher::PublisherImpl
{
public:
    /*
    /// C'tor
    PublisherImpl(std::shared_ptr<zmq::context_t> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger,
                  int )
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
    /// Disconnect
    void disconnect()
    {
        if (mBound)
        {
            mPublisher->disconnect(mAddress);
            mSocketDetails.clear();
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
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mPublisher{nullptr};
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    PublisherOptions mOptions;
    UCI::SocketDetails::Publisher mSocketDetails;
    std::string mAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
    bool mBound = false;
    bool mInitialized = false;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> (nullptr, nullptr))
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

/// C'tor with logger
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (nullptr, logger))
{
}

/// C'tor with context and logger
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

/// Move c'tor
Publisher::Publisher(Publisher &&publisher) noexcept
{
    *this = std::move(publisher);
}

/// Destructor
Publisher::~Publisher() = default;

/// Move assignment
Publisher& Publisher::operator=(Publisher &&publisher) noexcept
{
    if (&publisher == this){return *this;}
    pImpl = std::move(publisher.pImpl);
    return *this;
}

/// Initialize
void Publisher::initialize(const PublisherOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set on options");
    }
    pImpl->mOptions = options;
    pImpl->mInitialized = false;
    // Disconnnect from old connections
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
    // (Re)establish connections
    auto address = options.getAddress();
    try
    {   
        pImpl->mPublisher->bind(address);
    }   
    catch (const std::exception &e) 
    {
        auto errmsg = "Publisher failed to bind with error: "
                    + std::string(e.what());
        pImpl->mLogger->error(errmsg);
        throw std::runtime_error(errmsg);
    }   
    // Resolve the end point
    pImpl->mAddress = address;
    if (address.find("tcp") != std::string::npos ||
        address.find("ipc") != std::string::npos)
    {
        pImpl->mAddress = pImpl->mPublisher->get(zmq::sockopt::last_endpoint);
    }   
    pImpl->mBound = true;
    // Set some final details
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    pImpl->updateSocketDetails();
    pImpl->mInitialized = true;
}

/// Initialized?
bool Publisher::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Endpoint
std::string Publisher::getEndPoint() const
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    return pImpl->mAddress;
}

/// Send a message
void Publisher::send(const MessageFormats::IMessage &message)
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    auto messageType = message.getMessageType();
    if (messageType.empty())
    {
        pImpl->mLogger->debug("Message type is empty");
    }
    //auto cborMessage = std::string(message.toCBOR());
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

UCI::SocketDetails::Publisher Publisher::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Publisher not initialized");
    }
    return pImpl->mSocketDetails;
}
