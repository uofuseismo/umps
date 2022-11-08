#include <string>
#include <thread>
#include <mutex>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/logging/standardOut.hpp"
#include "private/messaging/requestReplySocket.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Request::RequestImpl : public ::RequestSocket
{
public:
    /// C'tor
    RequestImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
                const std::shared_ptr<UMPS::Logging::ILog> &logger) :
        RequestSocket(context, logger)
    {
    }
//private:
    UMPS::MessageFormats::Messages mMessageFormats;
    RequestOptions mOptions;
    UCI::SocketDetails::Request mSocketDetails;
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

Request::Request(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RequestImpl> (context, nullptr)) 
{
}

Request::Request(std::shared_ptr<UMPS::Messaging::Context> &context,
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
    // Extract options for populating the socket info 
    UMPS::Messaging::SocketOptions socketOptions;
    socketOptions.setAddress(options.getAddress());
    socketOptions.setMessageFormats(options.getMessageFormats());
    socketOptions.setZAPOptions(pImpl->mOptions.getZAPOptions());
    socketOptions.setSendHighWaterMark(options.getSendHighWaterMark());
    socketOptions.setReceiveHighWaterMark(options.getReceiveHighWaterMark());
    socketOptions.setSendTimeOut(options.getSendTimeOut());
    socketOptions.setReceiveTimeOut(options.getReceiveTimeOut());
    // Connect
    pImpl->connect(socketOptions);
    // Copy the options
    pImpl->mOptions = options; 
}

/// Initialized?
bool Request::isInitialized() const noexcept
{
    return pImpl->isConnected();
}

/// Make a request
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Request::request(const UMPS::MessageFormats::IMessage &request)
{
    if (!isInitialized()){throw std::runtime_error("Not initialized");}
    auto message = pImpl->request(request);
    if (message == nullptr)
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return message;
}

/// Disconnect
void Request::disconnect()
{
    pImpl->disconnect();
}

/// Destructor 
Request::~Request() = default;

/// Connection details
UCI::SocketDetails::Request Request::getSocketDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Request not initialized");
    }
    return pImpl->getSocketDetails();
}
