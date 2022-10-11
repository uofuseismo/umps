#include <iostream>
#include <thread>
#include "umps/proxyServices/command/remoteReplier.hpp"
#include "umps/proxyServices/command/remoteReplierOptions.hpp"
#include "umps/proxyServices/command/registrationRequest.hpp"
#include "umps/proxyServices/command/registrationResponse.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/logging/log.hpp"
//#include "private/messaging/replySocket.hpp"
#include "private/messaging/requestReplySocket.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyServices::Command;
namespace URouterDealer = UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;

/// This is a router-router pattern so we put a request (or dealer) socket on
/// the backend.
class RemoteReplier::RemoteReplierImpl : public ::RequestReplySocket
{
public:
    RemoteReplierImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                      std::shared_ptr<UMPS::Logging::ILog> logger) :
        ::RequestReplySocket(zmq::socket_type::dealer, context, logger)
    {
        // Module details
        mModuleDetails.setName("asdf");
        
        std::unique_ptr<UMPS::MessageFormats::IMessage> registrationResponse
            =  std::make_unique<RegistrationResponse> ();
        mMessageFormats.add(registrationResponse);
        
    }
    /// @brief This performs the polling loop that will:
    ///        1.  Wait for messages from the client.
    ///        2.  Call the callback to process the message.
    ///        3.  Return the result of the callback to the client.
    void poll() override
    {   
        if (!mHaveCallback)
        {
            throw std::runtime_error("Callback not set for poll");
        }
        mLogger->debug("Reply starting poll loop...");
        auto logLevel = mLogger->getLevel();
        while (isRunning())
        {
            // Poll
            std::array<zmq::pollitem_t, 1> pollItems
            {
                 {{mSocket->handle(), 0, ZMQ_POLLIN, 0}} 
            };
            zmq::poll(pollItems.data(), pollItems.size(), mPollTimeOut);
            // Got something
            if (pollItems[0].revents & ZMQ_POLLIN)
            {
                // Get the next message
                zmq::multipart_t messagesReceived(*mSocket);
                if (messagesReceived.empty()){continue;} // Deal with empty message
                if (logLevel >= UMPS::Logging::Level::Debug)
                {
                    mLogger->debug("Reply received message!");
                }
                if (messagesReceived.size() != 5)
                {
                    mLogger->error("Only 5-part messages handled");
#ifndef NDEBUG
                    assert(messagesReceived.size() == 5);
#endif
                    continue;
                }
                std::string messageType = messagesReceived.at(3).to_string();
                auto messageContents = reinterpret_cast<const void *>
                                       (messagesReceived.at(4).data());
                auto messageSize = messagesReceived.at(4).size();
                std::string responseMessageType;
                std::string responseMessage;
                try
                {
                    auto response = mCallback(messageType,
                                              messageContents,
                                              messageSize);
                    if (response != nullptr)
                    {
                        try
                        {
                            responseMessageType = response->getMessageType();
                            responseMessage = response->toMessage();
                            send(responseMessageType, responseMessage);
                        }
                        catch (const std::exception &e)
                        {
                            mLogger->error("Failed to send reply. Failed with: "
                                         + std::string{e.what()});
                        }
                    }
                    else
                    {
                        mLogger->error("Response is NULL check calllback");
                    }
                }
                catch (const std::exception &e)
                {
                    mLogger->error("Error in callback/serialization: "
                                 + std::string{e.what()});
                }
            }  // End check on poll
        } // End loop
        mLogger->debug("Reply poll loop finished");
    }
    UMPS::MessageFormats::Messages mMessageFormats;
    RemoteReplierOptions mOptions;
    ModuleDetails mModuleDetails;
    bool mRegistered{false};
};

/// C'tor
RemoteReplier::RemoteReplier() :
    pImpl(std::make_unique<RemoteReplierImpl> (nullptr, nullptr))
{
}

RemoteReplier::RemoteReplier(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RemoteReplierImpl> (context, nullptr))
{
}

RemoteReplier::RemoteReplier(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteReplierImpl> (nullptr, logger))
{
}

RemoteReplier::RemoteReplier(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteReplierImpl> (context, logger))
{
}

/// Destructor
RemoteReplier::~RemoteReplier() = default;

/// Initialize
void RemoteReplier::initialize(const RemoteReplierOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
    auto replyOptions = options.getOptions();
    UMPS::Messaging::SocketOptions socketOptions;
    socketOptions.setAddress(replyOptions.getAddress());
    socketOptions.setZAPOptions(replyOptions.getZAPOptions());
    socketOptions.setReceiveHighWaterMark(replyOptions.getHighWaterMark());
    socketOptions.setSendHighWaterMark(replyOptions.getHighWaterMark());
    socketOptions.setReceiveTimeOut(std::chrono::milliseconds {1000});//replyOptions.getTimeOut());
    //socketOptions.setSendTimeOut(replyOptions.getTimeOut());
    socketOptions.setMessageFormats(pImpl->mMessageFormats);
    socketOptions.setCallback(replyOptions.getCallback());
    pImpl->connectOrBind(socketOptions, true);
    pImpl->mOptions = options;
    pImpl->mLogger->debug("Module successfully connected!");
    // Attempt to register the module
    pImpl->mLogger->debug("Registering module...");
    RegistrationRequest request;
    request.setModuleDetails(pImpl->mModuleDetails);
    pImpl->send(request); // Send my details
    auto replyMessage = pImpl->receive(); // Wait for a reply
    if (replyMessage == nullptr)
    {
        throw std::runtime_error("Registration request timed-out");
    }
    auto response
        = static_unique_pointer_cast<RegistrationResponse>
          (std::move(replyMessage));
    if (response->getReturnCode() != RegistrationReturnCode::Success)
    {
        throw std::runtime_error("Failed to register module");
    }
    pImpl->mRegistered = true;
    pImpl->mLogger->debug("Module successfully registered!");
}

/// Initialized
bool RemoteReplier::isInitialized() const noexcept
{
    return pImpl->isConnected() && pImpl->mRegistered;
}

/// Start
void RemoteReplier::start()
{
    if (!isInitialized()){throw std::runtime_error("Replier not initialized");}
    pImpl->start();
}

/// Stop
void RemoteReplier::stop()
{
    pImpl->stop();
}

/// Connection information
/*
UCI::SocketDetails::Reply RemoteReplier::getSocketDetails() const
{
    //return pImpl->getSocketDetails();
}
*/
