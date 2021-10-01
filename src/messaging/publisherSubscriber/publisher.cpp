#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/authentication/enums.hpp"
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
    PublisherImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    /// C'tor
    explicit PublisherImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
            mPublisher = std::make_unique<zmq::socket_t> (*mContext,
                                                         zmq::socket_type::pub);
        }
    }
    /// C'tor
    explicit PublisherImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// C'tor
    PublisherImpl(std::shared_ptr<zmq::context_t> &context,
                  std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(context),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(logger)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
            mPublisher = std::make_unique<zmq::socket_t> (*mContext,
                                                         zmq::socket_type::pub);
        }
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mPublisher;
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    Authentication::SecurityLevel mSecurityLevel
        = Authentication::SecurityLevel::GRASSLANDS;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> ())
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context) :
   pImpl(std::make_unique<PublisherImpl> (context))
{
}

/// C'tor with logger
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (logger))
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
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    if (isAuthenticationServer)
    {
        pImpl->mPublisher->set(zmq::sockopt::zap_domain, domain); 
    }
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
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    if (!isAuthenticationServer)
    {
        if (!credentials.haveUserName())
        {
            throw std::invalid_argument("Username must be set for ZAP client");
        }
        if (!credentials.havePassword())
        {
            throw std::invalid_argument("Password must be set for ZAP client");
        }
    }
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    pImpl->mPublisher->set(zmq::sockopt::zap_domain, domain);
    if (!isAuthenticationServer)
    {
        pImpl->mPublisher->set(zmq::sockopt::plain_server, 0);
        pImpl->mPublisher->set(zmq::sockopt::plain_username,
                               credentials.getUserName());
        pImpl->mPublisher->set(zmq::sockopt::plain_password,
                               credentials.getPassword());
    }
    else
    {
        pImpl->mPublisher->set(zmq::sockopt::plain_server, 1);
    }
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
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!serverKeys.havePrivateKey())
    {
        throw std::invalid_argument("Server private key not set");
    }
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    pImpl->mPublisher->set(zmq::sockopt::zap_domain, domain);
    pImpl->mPublisher->set(zmq::sockopt::curve_server, 1);
    auto serverKey = serverKeys.getPublicTextKey();
    auto privateKey = serverKeys.getPrivateTextKey();
    pImpl->mPublisher->set(zmq::sockopt::curve_publickey, serverKey.data());
    pImpl->mPublisher->set(zmq::sockopt::curve_secretkey, privateKey.data()); 

    pImpl->mPublisher->bind(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
}

/// Connect subscriber as a CURVE client 
void Publisher::bind(
    const std::string &endPoint,
    const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
    const UMPS::Messaging::Authentication::Certificate::Keys &clientKeys,
    const std::string &domain)
{
    if (isEmpty(endPoint)){throw std::invalid_argument("endPoint is empty");}
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!clientKeys.havePublicKey())
    {
        throw std::invalid_argument("Client public key not set");
    }
    if (!clientKeys.havePrivateKey())
    {
        throw std::invalid_argument("Client private key not set");
    }
    if (pImpl->mEndPoints.contains(endPoint))
    {
        throw std::runtime_error("Already bound to endpoint: " + endPoint);
    }
    pImpl->mPublisher->set(zmq::sockopt::zap_domain, domain);
    pImpl->mPublisher->set(zmq::sockopt::curve_server, 0); 
    auto serverPublicKey  = serverKeys.getPublicTextKey();
    auto clientPublicKey  = clientKeys.getPublicTextKey();
    auto clientPrivateKey = clientKeys.getPrivateTextKey();
    pImpl->mPublisher->set(zmq::sockopt::curve_serverkey,
                            serverPublicKey.data());
    pImpl->mPublisher->set(zmq::sockopt::curve_publickey,
                            clientPublicKey.data());
    pImpl->mPublisher->set(zmq::sockopt::curve_secretkey,
                            clientPrivateKey.data());
    pImpl->mPublisher->bind(endPoint);
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
}

/// Send a message
void Publisher::send(const MessageFormats::IMessage &message)
{
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
