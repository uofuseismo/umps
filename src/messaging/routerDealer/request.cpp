#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/logging/stdout.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Request::RequestImpl
{
public:
    /// C'tor
    RequestImpl(std::shared_ptr<zmq::context_t> context,
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
        mClient = std::make_unique<zmq::socket_t> (*mContext,
                                                   zmq::socket_type::req);
    }
    /// Update socket details
    void updateSocketDetails()
    {   
        mSocketDetails.setAddress(mAddress);
        mSocketDetails.setSecurityLevel(mSecurityLevel);
        mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::BIND);
    }
//private:
    UMPS::MessageFormats::Messages mMessageFormats;
    RequestOptions mOptions;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mClient;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    std::string mAddress;
    UCI::SocketDetails::Request mSocketDetails;
    int mHighWaterMark = 200;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
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

Request::Request(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<RequestImpl> (context, nullptr))
{
}

Request::Request(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (context, logger))
{
}

/// Move c'tor
Request::Request(Request &&request) noexcept
{
    *this = std::move(request);
}

/// Move assignment
Request& Request::operator=(Request &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Initializes the router
void Request::initialize(const RequestOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    if (!options.haveMessageFormats())
    {
        throw std::invalid_argument("No message formats set");
    }
    pImpl->mMessageFormats = options.getMessageFormats();
    pImpl->mOptions.clear();
    disconnect();
    pImpl->mOptions = options;
    //  
    auto zapOptions = pImpl->mOptions.getZAPOptions();
    auto highWaterMark = pImpl->mOptions.getHighWaterMark();
    auto address = pImpl->mOptions.getAddress();
    // Set the ZAP options
    zapOptions.setSocketOptions(&*pImpl->mClient);
    pImpl->mSecurityLevel = zapOptions.getSecurityLevel();
    // Set the high water mark
    pImpl->mClient->set(zmq::sockopt::rcvhwm, highWaterMark);
    //pImpl->mClient->set(zmq::sockopt::sndhwm, highWaterMark); 
    //pImpl->mClient->set(zmq::sockopt::rcvtimeo, 100);
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
    pImpl->mConnected = true;
    pImpl->updateSocketDetails();
    pImpl->mInitialized = true;
}

/// Initialized?
bool Request::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

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
        pImpl->mSocketDetails.clear();
        pImpl->mAddress.clear();
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

/// Connection details
UCI::SocketDetails::Request Request::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Request not initialized");
    }
    return pImpl->mSocketDetails;
}
