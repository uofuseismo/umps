#include <iostream>
#include <thread>
#include "umps/proxyServices/command/replier.hpp"
#include "umps/proxyServices/command/replierOptions.hpp"
#include "umps/proxyServices/command/registrationRequest.hpp"
#include "umps/proxyServices/command/registrationResponse.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/failure.hpp"
#include "umps/messaging/context.hpp"
#include "umps/services/command/terminateRequest.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/logging/log.hpp"
#include "umps/messageFormats/staticUniquePointerCast.hpp"
//#include "private/messaging/replySocket.hpp"
#include "private/messaging/requestReplySocket.hpp"
#include "private/services/ping.hpp"
#include "private/services/terminate.hpp"

// Wait up to this many milliseconds for registration/deregistration prcoess
// to complete
#define REGISTRATION_TIME 5000

using namespace UMPS::ProxyServices::Command;
namespace URouterDealer = UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UMF = UMPS::MessageFormats;

/// This is a router-router pattern so we put a request (or dealer) socket on
/// the backend.
class Replier::ReplierImpl : public ::RequestReplySocket
{
public:
    ReplierImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                      std::shared_ptr<UMPS::Logging::ILog> logger) :
        ::RequestReplySocket(zmq::socket_type::dealer, context, logger)
    {
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
        ::PingRequest privatePingRequest;
        ::PingResponse privatePingResponse;
        ::TerminateRequest privateTerminateRequest;
        ::TerminateResponse privateTerminateResponse;
        UMPS::Services::Command::TerminateRequest terminateRequest;
        auto terminateRequestMessageType = terminateRequest.getMessageType();
        auto terminateRequestContents = terminateRequest.toMessage();
        mLogger->debug("Reply starting poll loop...");
        auto logLevel = mLogger->getLevel();
        while (isRunning())
        {
            // Poll
            std::array<zmq::pollitem_t, 1> pollItems
            {
                 {{mSocket->handle(), 0, ZMQ_POLLIN, 0}}
            };
            zmq::poll(pollItems.data(),
                      pollItems.size(),
                      std::chrono::milliseconds {10});
            // Got something
            if (pollItems[0].revents & ZMQ_POLLIN)
            {
                // Get the next message
                zmq::multipart_t messagesReceived(*mSocket);
                // Deal with empty message
                if (messagesReceived.empty()){continue;}
                if (logLevel >= UMPS::Logging::Level::Debug)
                {
                    mLogger->debug("Replier received message!");
                }
                // To be consistent with ZeroMQ manual I'm expecting:
                // 1. Client identity
                // 2. Empty
                // 3. Message [Header + Body so actually 2 things]
                if (messagesReceived.size() != 4)
                {
                    mLogger->error("Only 4-part messages handled");
#ifndef NDEBUG
                    assert(messagesReceived.size() == 4);
#endif
                    continue;
                }
                auto returnAddress = messagesReceived.at(0).to_string(); 
                auto messageType = messagesReceived.at(2).to_string();
                auto messageContents = reinterpret_cast<const void *>
                                       (messagesReceived.at(3).data());
                auto messageSize = messagesReceived.at(3).size();
                std::string responseMessageType;
                std::string responseMessage;
                // Handle ping request
                if (messageType == privatePingRequest.getMessageType())
                {
                    privatePingRequest.fromMessage(
                       reinterpret_cast<const char *> (messageContents),
                          messageSize);
                    privatePingResponse.setTime(privatePingRequest.getTime());
                    auto response = privatePingResponse.clone();
                    try
                    {
                        zmq::multipart_t reply;
                        reply.addstr(response->getMessageType());
                        reply.addstr(response->toMessage());
                        reply.send(*mSocket);
                    }
                    catch (const std::exception &e) 
                    {
                        mLogger->error(
                            "Failed to send ping reply. Failed with: "
                           + std::string{e.what()});
                    }
                }
                else if (messageType ==
                         privateTerminateRequest.getMessageType())
                {
                    mLogger->info("Terminate request received from proxy.");
                    privateTerminateRequest.fromMessage(
                       reinterpret_cast<const char *> (messageContents),
                          messageSize);
                    // Submit a terminate request to the callback.
                    // If I can't process the command then it's own my problem.
                    // I just want to let the proxy know that I received the
                    // message and attempted to process it.
                    try
                    {
                        auto response = mCallback(terminateRequestMessageType,
                                               terminateRequestContents.data(),
                                               terminateRequestContents.size());
                        if (response == nullptr)
                        {
                            mLogger->error("Callback didnt process terminate");
                        }                                       
                    }
                    catch (const std::exception &e)
                    {
                        mLogger->error("Callback threw on terminate: "
                                     + std::string{e.what()});
                    }
                    // Send custom terminate response back
                    try
                    {
                        auto response = privateTerminateResponse.clone();
                        zmq::multipart_t reply;
                        reply.addstr(response->getMessageType());
                        reply.addstr(response->toMessage());
                        reply.send(*mSocket);
                    }
                    catch (const std::exception &e)
                    {
                        mLogger->error(
                            "Failed to send ping reply. Failed with: "
                           + std::string{e.what()});
                    }
                }
                else
                {
                    std::unique_ptr<UMPS::MessageFormats::IMessage>
                        response{nullptr};
                    try
                    {
                        response = mCallback(messageType,
                                             messageContents,
                                             messageSize);
                    }
                    catch (const std::exception &e) 
                    {
                        mLogger->error("Error in callback/serialization: "
                                     + std::string{e.what()});
                        // The user's callbacks should not fail.  But, just
                        // in case, send something back.
                        UMPS::MessageFormats::Failure failureMessage;
                        failureMessage.setDetails("Error in callback");
                        response = failureMessage.clone();
                    }
                    if (response != nullptr)
                    {
                        try
                        {
                            zmq::multipart_t reply;
                            reply.addstr(returnAddress);
                            reply.addstr("");
                            reply.addstr(response->getMessageType());
                            reply.addstr(response->toMessage());
                            reply.send(*mSocket);
                        }
                        catch (const std::exception &e)
                        {
                            mLogger->error(
                               "Failed to send reply. Failed with: "
                              + std::string{e.what()});
                        }
                    }
                    else
                    {
                        mLogger->error("Response is NULL check calllback");
                    }
                }
            }  // End check on poll
        } // End loop
        mLogger->debug("Reply poll loop finished");
    }
    UMPS::MessageFormats::Messages mMessageFormats;
    ReplierOptions mOptions;
    ModuleDetails mModuleDetails;
    bool mRegistered{false};
};

/// C'tor
Replier::Replier() :
    pImpl(std::make_unique<ReplierImpl> (nullptr, nullptr))
{
}

Replier::Replier(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ReplierImpl> (context, nullptr))
{
}

Replier::Replier(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierImpl> (nullptr, logger))
{
}

Replier::Replier(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierImpl> (context, logger))
{
}

/// Destructor
Replier::~Replier()
{
    stop();
}

/// Initialize
void Replier::initialize(const ReplierOptions &options)
{
    if (!options.haveModuleDetails())
    {
        throw std::invalid_argument("Module details not set");
    }
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
    auto replyOptions = options.getOptions();
    auto moduleDetails = options.getModuleDetails();
    UMPS::Messaging::SocketOptions socketOptions;
    socketOptions.setAddress(replyOptions.getAddress());
    socketOptions.setZAPOptions(replyOptions.getZAPOptions());
    socketOptions.setReceiveHighWaterMark(replyOptions.getHighWaterMark());
    socketOptions.setSendHighWaterMark(0); // Cache all responses
    //socketOptions.setLingerPeriod(std::chrono::milliseconds {10});
    // Polling loop so return immediately
    socketOptions.setReceiveTimeOut(std::chrono::milliseconds {0});
    // Send immediately
    socketOptions.setSendTimeOut(std::chrono::milliseconds {0}); 
    //socketOptions.setSendTimeOut(replyOptions.getTimeOut());
    socketOptions.setMessageFormats(pImpl->mMessageFormats);
    socketOptions.setCallback(replyOptions.getCallback());
    pImpl->connectOrBind(socketOptions, true);
    pImpl->mOptions = options;
    pImpl->mLogger->debug("Module successfully connected!");
    // Attempt to register the module
    pImpl->mLogger->debug("Registering module...");
    RegistrationRequest request;
    request.setModuleDetails(moduleDetails);
    request.setRegistrationType(RegistrationType::Register);
    pImpl->send(request); // Send my request
    // Wait (up to 5 seconds) for a registration response.
    auto receiveTimeOut = socketOptions.getReceiveTimeOut();
    pImpl->mSocket->set(zmq::sockopt::rcvtimeo, REGISTRATION_TIME);
    auto replyMessage = pImpl->receive(); // Wait for a reply
    pImpl->mSocket->set(zmq::sockopt::rcvtimeo,
                        static_cast<int> (receiveTimeOut.count()));
    if (replyMessage == nullptr)
    {
        throw std::runtime_error("Registration request timed-out");
    }
    auto response
        = UMF::static_unique_pointer_cast<RegistrationResponse>
          (std::move(replyMessage));
    if (response->getReturnCode() != RegistrationReturnCode::Success)
    {
        throw std::runtime_error("Failed to register module");
    }
    pImpl->mModuleDetails = moduleDetails;
    pImpl->mRegistered = true;
    pImpl->mLogger->debug("Module successfully registered!");
}

/// Initialized
bool Replier::isInitialized() const noexcept
{
    return pImpl->isConnected() && pImpl->mRegistered;
}

/// Start
void Replier::start()
{
    if (!isInitialized()){throw std::runtime_error("Replier not initialized");}
    pImpl->start();
}

/// Stop
void Replier::stop()
{
    // Make sure the poller is stopped so we don't pick up the unsubscribe
    // message.
    pImpl->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds {100});
    // Deregister the module
    if (pImpl->isConnected() && pImpl->mModuleDetails.haveName())
    {
        pImpl->mLogger->debug("Deregistering module...");
        // Create deregistration request
        RegistrationRequest request;
        request.setModuleDetails(pImpl->mModuleDetails);
        request.setRegistrationType(RegistrationType::Deregister);
        pImpl->send(request); // Send my reuest
        // Override my timeout
        auto receiveTimeOut = pImpl->mSocket->get(zmq::sockopt::rcvtimeo);
        pImpl->mSocket->set(zmq::sockopt::rcvtimeo, REGISTRATION_TIME);
        // There's a chance some old un-processed message is laying around.
        // In this case just send the deregistration message.  It's likely
        // we succeeded but we just read the wrong message.
        try
        {
            RegistrationResponse registrationResponse;
            bool gotDeregistrationResponse{false};
            while (true)
            {
                auto msg = pImpl->receiveZMQ();
                if (msg.empty()){break;}
                for (const auto &m : msg)
                {
                    std::string contents{
                       reinterpret_cast<const char *> (m.data()), m.size()};
                    if (contents == registrationResponse.getMessageType())
                    {
                        gotDeregistrationResponse = true;
                        break;
                    }
                }
                if (gotDeregistrationResponse){break;}
            }
            if (!gotDeregistrationResponse)
            {
                pImpl->mLogger->warn("Deregistration request timed-out");
            }
            else
            {
                pImpl->mLogger->debug("Module deregistered!");
            }
        }
        catch (const std::exception &e)
        {
            pImpl->mLogger->error("Failed read deregister response. Failed with"
                                + std::string {e.what()});
        }
        pImpl->mSocket->set(zmq::sockopt::rcvtimeo, receiveTimeOut);
        pImpl->mModuleDetails.clear(); // Done
    }
}

/// Connection information
UCI::SocketDetails::Request Replier::getSocketDetails() const
{
    return pImpl->mRequestSocketDetails;
}

/// Running?
bool Replier::isRunning() const noexcept
{
    return pImpl->isRunning();
}
