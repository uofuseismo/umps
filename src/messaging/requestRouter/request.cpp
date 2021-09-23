#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/logging/stdout.hpp"

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
    std::map<std::string, std::unique_ptr<UMPS::MessageFormats::IMessage>> 
        mSubscriptions;
    std::shared_ptr<zmq::context_t> mContext = nullptr;
    std::unique_ptr<zmq::socket_t> mClient;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    std::string mEndpoint;
    int mHighWaterMark = 200;
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

/// Connect
void Request::connect(const std::string &endpoint)
{
    disconnect(); // Hang up before reconnecting
    try
    {
        pImpl->mLogger->debug("Attempting to connect to: " + endpoint);
        pImpl->mClient->connect(endpoint);
        pImpl->mClient->set(zmq::sockopt::rcvhwm, pImpl->mHighWaterMark);
        pImpl->mEndpoint = endpoint;
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
    auto idx = pImpl->mSubscriptions.find(messageType);
    if (idx == pImpl->mSubscriptions.end())
    {
        pImpl->mLogger->debug("Adding subscription: " + messageType);
        pImpl->mSubscriptions.insert(std::pair(messageType,
                                               std::move(message)));
    }
    else
    {
        pImpl->mLogger->debug("Overwriting subscription: " + messageType);
        idx->second = std::move(message);
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
    auto requestMessage = request.toCBOR();
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
    std::vector<zmq::message_t> responseReceived;
    zmq::recv_result_t receivedResult =
        zmq::recv_multipart(*pImpl->mClient,
                            std::back_inserter(responseReceived));
    pImpl->mLogger->debug("Response received; unpacking...");
//std::cout << *receivedResult << std::endl;
//std::cout << responseReceived.at(0).to_string() << std::endl;
//std::cout << responseReceived.size() << std::endl;
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
    auto index = pImpl->mSubscriptions.find(responseMessageType);
    if (index == pImpl->mSubscriptions.end())
    {
        throw std::runtime_error("Unhandled response type");
    } 
    const auto payload = static_cast<uint8_t *> (responseReceived.at(1).data());
    auto responseLength = responseReceived.at(1).size();
    auto response = index->second->createInstance();
    try
    {
        response->fromCBOR(payload, responseLength);
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


