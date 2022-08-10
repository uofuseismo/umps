#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RequestRouter;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Request::RequestImpl
{
public:
    /*
    /// C'tor
    RequestImpl(std::shared_ptr<zmq::context_t> context,
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
        mClient = std::make_unique<zmq::socket_t> (*mContext,
                                                   zmq::socket_type::req);
    }
    */
    /// C'tor
    RequestImpl(std::shared_ptr<UMPS::Messaging::Context> context,
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
        mClient = std::make_unique<zmq::socket_t> (*contextPtr,
                                                   zmq::socket_type::req);
    }
    /// Update socket details
    void updateSocketDetails()
    {   
        mSocketDetails.setAddress(mAddress);
        mSocketDetails.setSecurityLevel(mSecurityLevel);
        mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::Bind);
    }
//private:
    //std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
    //    mSubscriptions;
    UMPS::MessageFormats::Messages mMessageFormats;
    RequestOptions mOptions;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mClient{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::string mAddress;
    UCI::SocketDetails::Request mSocketDetails;
    int mHighWaterMark = 200;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::Grasslands;
    bool mInitialized = false;
    bool mConnected = false;
};

/// C'tor
Request::Request() :
    pImpl(std::make_unique<RequestImpl> (nullptr, nullptr))
{
}

Request::Request(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (nullptr, logger))
{
}

/*
Request::Request(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<RequestImpl> (context, nullptr, 0))
{
}
*/

Request::Request(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RequestImpl> (context, nullptr)) 
{
}

/*
Request::Request(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (context, logger, 0))
{
}
*/

Request::Request(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (context, logger)) 
{
}

/// Initializes the router
void Request::initialize(const RequestOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("End point not set");
    }
    pImpl->mMessageFormats = options.getMessageFormats();
    if (pImpl->mMessageFormats.empty())
    {
        pImpl->mLogger->warn("No message types set in options");
    }
    pImpl->mOptions.clear();
    disconnect();
    pImpl->mOptions = options;
    //  
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    auto highWaterMark = pImpl->mOptions.getHighWaterMark();
    auto address = pImpl->mOptions.getAddress();
    auto timeOut = static_cast<int> (options.getTimeOut().count());
    // Set the ZAP options
    zapOptions.setSocketOptions(&*pImpl->mClient);
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    // Set the high water mark
    pImpl->mClient->set(zmq::sockopt::rcvhwm, highWaterMark);
    //pImpl->mClient->set(zmq::sockopt::sndhwm, highWaterMark); 
    // Set the timeout
    if (timeOut >= 0)
    {
        pImpl->mClient->set(zmq::sockopt::rcvtimeo, timeOut);
    }
    // Bind
    pImpl->mLogger->debug("Attempting to connect to: " + address);
    pImpl->mClient->connect(address);
    pImpl->mLogger->debug("Connected to: " + address + "!");
    // Resolve end point
    pImpl->mAddress = address;
    if (address.find("tcp") != std::string::npos ||
        address.find("ipc") != std::string::npos)
    {
        pImpl->mAddress = pImpl->mClient->get(zmq::sockopt::last_endpoint);
    }
    pImpl->updateSocketDetails();
    pImpl->mConnected = true;
    pImpl->mInitialized = true;
}

/// Initialized?
bool Request::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/*
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
    if (pImpl->mMessageFormats.contains(messageType))
    {
        pImpl->mLogger->debug("Message type: " + messageType
                            + " alread exists");
    }
    else
    {
        pImpl->mMessageFormats.add(message);
    }
}
*/

/// Make a request
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Request::request(const UMPS::MessageFormats::IMessage &request)
{
    if (!isInitialized()){throw std::runtime_error("Not initialized");}
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
    if (responseReceived.empty()){return nullptr;} // Timeout
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
    if (!pImpl->mMessageFormats.contains(responseMessageType))
    {
        throw std::runtime_error("Unhandled response type: "
                               + responseMessageType);
    }
    const auto payload = static_cast<char *> (responseReceived.at(1).data());
    auto responseLength = responseReceived.at(1).size();
    auto response = pImpl->mMessageFormats.get(responseMessageType);
    try
    {
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
    if (pImpl->mConnected)
    {
        pImpl->mLogger->debug("Disconnecting from " + pImpl->mAddress);
        pImpl->mClient->disconnect(pImpl->mAddress);
        pImpl->mAddress.clear();
        pImpl->mSocketDetails.clear();
        pImpl->mConnected = false;
    }
}

/// Destructor 
Request::~Request() = default;

/// Security level
UAuth::SecurityLevel Request::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Connection information
UCI::SocketDetails::Request Request::getSocketDetails() const
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Request not initialized");
    }   
    return pImpl->mSocketDetails;
}

