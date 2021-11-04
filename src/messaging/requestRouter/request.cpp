#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"
#include "private/authentication/zapOptions.hpp"

using namespace UMPS::Messaging::RequestRouter;

class Request::RequestImpl
{
public:
    /// C'tor
    RequestImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mClient(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::req)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()) 
    {   
    }   
    /// C'tor with specified logger
    explicit RequestImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mClient(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::req)),
        mLogger(logger)
    {   
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
    }
//private:
    //std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
    //    mSubscriptions;
    UMPS::MessageFormats::Messages mMessages;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mClient;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    std::string mEndpoint;
    int mHighWaterMark = 200;
    Authentication::SecurityLevel mSecurityLevel
        = Authentication::SecurityLevel::GRASSLANDS;
    bool mConnected = false;
};

/// C'tor
Request::Request() :
    pImpl(std::make_unique<RequestImpl> ())
{
}

/// C'tor
Request::Request(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (logger))
{
}

/// Connect grasslands
void Request::connect(const std::string &endpoint)
{
    if (isEmpty(endpoint)){throw std::invalid_argument("Endpoint is empty");}
    disconnect(); // Hang up before reconnecting
    try
    {
        pImpl->mLogger->debug("Attempting to connect to: " + endpoint);
        pImpl->mClient->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
        pImpl->mClient->connect(endpoint);
        pImpl->mEndpoint = endpoint;
        pImpl->mSecurityLevel = Authentication::SecurityLevel::GRASSLANDS;
        pImpl->mConnected = true;
        pImpl->mLogger->debug("Connected to: " + endpoint + "!");
    }
    catch (const std::exception &e)
    {
        auto error = "Failed to connect to: " + endpoint
                   + " with\n" + e.what();
        pImpl->mLogger->error(error);
    }
}

/// Connect strawhouse
void Request::connect(const std::string &endpoint,
                      const bool isAuthenticationServer,
                      const std::string &zapDomain)
{
    if (isEmpty(endpoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    disconnect(); // Hangup
    // ZAP
    setStrawhouse(pImpl->mClient.get(), isAuthenticationServer, zapDomain);

    pImpl->mClient->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);

    pImpl->mClient->connect(endpoint);
    pImpl->mEndpoint = endpoint;
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STRAWHOUSE;
    pImpl->mConnected = true;
}

/// Connect woodhouse
void Request::connect(
    const std::string &endpoint,
    const Authentication::Certificate::UserNameAndPassword &credentials,
    const bool isAuthenticationServer,
    const std::string &zapDomain)
{
    if (isEmpty(endpoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!isAuthenticationServer)
    {
        if (!credentials.haveUserName())
        {
            throw std::invalid_argument("Username not set");
        }
        if (!credentials.havePassword())
        {
            throw std::invalid_argument("Password not set");
        }
    }
    disconnect(); // Hangup
    // Authentication server?
    setWoodhouse(pImpl->mClient.get(), credentials,
                 isAuthenticationServer, zapDomain);

    pImpl->mClient->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);

    pImpl->mClient->connect(endpoint);
    pImpl->mEndpoint = endpoint;
    pImpl->mSecurityLevel = Authentication::SecurityLevel::WOODHOUSE;
    pImpl->mConnected = true;
}

/// Connect stonehouse CURVE server
void Request::connect(const std::string &endpoint,
                      const Authentication::Certificate::Keys &serverKeys,
                      const std::string &zapDomain)
{
    if (isEmpty(endpoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!serverKeys.havePrivateKey())
    {
        throw std::invalid_argument("Server private key not set");
    }
    disconnect(); // Hangup
    // Set ZAP 
    setStonehouseServer(pImpl->mClient.get(), serverKeys, zapDomain);

    pImpl->mClient->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);

    pImpl->mClient->connect(endpoint);
    pImpl->mEndpoint = endpoint;
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
    pImpl->mConnected = true;
}

/// Connect stonehouse CURVE client
void Request::connect(const std::string &endpoint,
                      const Authentication::Certificate::Keys &serverKeys,
                      const Authentication::Certificate::Keys &clientKeys,
                      const std::string &zapDomain)
{
    if (isEmpty(endpoint)){throw std::invalid_argument("Endpoint is empty");}
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
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
    disconnect(); // Hangup
    // Set ZAP protocol
    setStonehouseClient(pImpl->mClient.get(), serverKeys,
                        clientKeys, zapDomain);

    pImpl->mClient->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);

    pImpl->mClient->connect(endpoint);
    pImpl->mEndpoint = endpoint;
    pImpl->mSecurityLevel = Authentication::SecurityLevel::STONEHOUSE;
    pImpl->mConnected = true;
}

/// Connected?
bool Request::isConnected() const noexcept
{
    return pImpl->mConnected;
}

/// Add a subscription
void Request::setResponse(
    std::unique_ptr<UMPS::MessageFormats::IMessage> &message)
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    auto messageType = message->getMessageType();
    if (messageType.empty())
    {
        throw std::invalid_argument("Message type is empty");
    }
    if (pImpl->mMessages.contains(messageType))
    {
        pImpl->mLogger->debug("Messaget type: " + messageType
                            + " alread exists");
    }
    else
    {
        pImpl->mMessages.add(message);
    }
}

/// Make a request
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Request::request(const UMPS::MessageFormats::IMessage &request)
{
    if (!isConnected()){throw std::runtime_error("Not connected");}
   
    auto messageType = request.getMessageType();
    if (messageType.empty())
    {
        pImpl->mLogger->error("Message type is empty");
    }
    //auto requestMessage = request.toCBOR();
    auto requestMessage = request.toMessage();
    if (requestMessage.empty())
    {
        pImpl->mLogger->error("Message contents is empty");
    }
    // Send the message
    pImpl->mLogger->debug("Sending message: " + messageType);
    zmq::const_buffer headerRequest{messageType.data(), messageType.size()};
    
    pImpl->mClient->send(headerRequest, zmq::send_flags::sndmore);
    zmq::const_buffer bufferRequest{requestMessage.data(),
                                    requestMessage.size()};
    pImpl->mClient->send(bufferRequest);
    // Wait for the response
    pImpl->mLogger->debug("Blocking for response...");
    // Receive all parts of the message
    zmq::multipart_t responseReceived(*pImpl->mClient);
#ifndef NDEBUG
    assert(responseReceived.size() == 2);
#else
    if (responseReceived.size() != 2)
    {   
        pImpl->mLogger->error("Only 2-part messages handled");
        throw std::runtime_error("Only 2-part messages handled");
    }
#endif
    // Unpack the response
    std::string responseMessageType = responseReceived.at(0).to_string();
    if (!pImpl->mMessages.contains(responseMessageType))
    {
        throw std::runtime_error("Unhandled response type: "
                               + responseMessageType);
    }
    //const auto payload = static_cast<uint8_t *> (responseReceived.at(1).data());
    const auto payload = static_cast<char *> (responseReceived.at(1).data());
    auto responseLength = responseReceived.at(1).size();
    auto response = pImpl->mMessages.get(responseMessageType);
    try
    {
        //response->fromCBOR(payload, responseLength);
        response->fromMessage(payload, responseLength);
    }
    catch (const std::exception &e) 
    {
        auto errorMsg = "Failed to unpack message of type: " + messageType;
        pImpl->mLogger->error(errorMsg);
        throw;
    }
    return response;
}

/// Disconnect
void Request::disconnect()
{
    if (isConnected())
    {
        pImpl->mLogger->debug("Disconnecting from " + pImpl->mEndpoint);
        pImpl->mClient->disconnect(pImpl->mEndpoint);
        pImpl->mEndpoint.clear();
        pImpl->mConnected = false;
    }
}

/// Destructor 
Request::~Request() = default;

/// Security level
UMPS::Messaging::Authentication::SecurityLevel
    Request::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}
