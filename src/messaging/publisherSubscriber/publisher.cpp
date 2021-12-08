#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/publisherSubscriber/publisherOptions.hpp"
#include "umps/messaging/authentication/enums.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;

class Publisher::PublisherImpl
{
public:
    /// C'tor
    PublisherImpl(std::shared_ptr<zmq::context_t> context,
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
        mPublisher = std::make_unique<zmq::socket_t> (*mContext,
                                                      zmq::socket_type::pub);
    }
    /// Disconnect
    void disconnect()
    {
        if (mBound)
        {
            mPublisher->disconnect(mAddress);
            mBound = false;
        }
    }
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mPublisher;
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    PublisherOptions mOptions;
    std::string mAddress;
    Authentication::SecurityLevel mSecurityLevel
        = Authentication::SecurityLevel::GRASSLANDS;
    bool mBound = false;
    bool mInitialized = false;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> (nullptr, nullptr))
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context) :
   pImpl(std::make_unique<PublisherImpl> (context, nullptr))
{
}

/// C'tor with logger
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (nullptr, logger))
{
}

/// C'tor with context and logger
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context,
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
    //auto timeOut = static_cast<int> (options.getTimeOut().count());
    auto hwm = pImpl->mOptions.getHighWaterMark();
    if (hwm > 0){pImpl->mPublisher->set(zmq::sockopt::sndhwm, hwm);}
    //if (timeOut >= 0)
    //{   
    //    pImpl->mPublisher->set(zmq::sockopt::sndtimeo, timeOut);
    //}   
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

/*
/// Bind to an address if not already done so
void Publisher::bind(const std::string &endPoint)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
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
        pImpl->mPublisher->bind(endPoint);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Failed to bind to endpoint: " + endPoint
                      + "\nZeroMQ failed with: " + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::GRASSLANDS;
}

/// Strawhouse
void Publisher::bind(
    const std::string &endPoint,
    const bool isAuthenticationServer,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setStrawhouse(pImpl->mPublisher.get(), isAuthenticationServer, domain);
    pImpl->mPublisher->bind(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STRAWHOUSE;
}


/// Woodhouse
void Publisher::bind(
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
    setWoodhouse(pImpl->mPublisher.get(), credentials,
                 isAuthenticationServer, domain);
    pImpl->mPublisher->bind(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::WOODHOUSE;
}

/// Stonehouse server
void Publisher::bind(
    const std::string &endPoint,
    const Authentication::Certificate::Keys &serverKeys,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setStonehouseServer(pImpl->mPublisher.get(), serverKeys, domain);
    pImpl->mPublisher->bind(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
}

/// Connect publisher as a CURVE client 
void Publisher::bind(
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
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    setStonehouseClient(pImpl->mPublisher.get(),
                        serverKeys, clientKeys, domain);
    pImpl->mPublisher->bind(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
}
*/

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
UMPS::Messaging::Authentication::SecurityLevel 
    Publisher::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}
