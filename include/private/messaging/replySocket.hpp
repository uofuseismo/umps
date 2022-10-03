#ifndef PRIVATE_MESSAGING_REPLY_SOCKET_HPP
#define PRIVATE_MESSAGING_REPLY_SOCKET_HPP
#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include <string>
#include <array>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/authentication/zapOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "private/messaging/ipcDirectory.hpp"
namespace
{
/// @brief This is a base implementation for a reply socket.
class ReplySocket
{
public:
    /// @brief C'tor
    ReplySocket(std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                std::shared_ptr<UMPS::Logging::ILog> logger = nullptr)
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
        mServer = std::make_unique<zmq::socket_t> (*contextPtr,
                                                   zmq::socket_type::rep);
    }
    /// @brief Destructor
    virtual ~ReplySocket()
    {
        disconnect();
    }
    /// @brief Disconnect
    virtual void disconnect()
    {
        stop(); // Make sure service thread stopped before disconnecting
        if (mConnected)
        {
            mServer->disconnect(mAddress);
            ::removeIPCFile(mAddress, &*mLogger);
            mAddress.clear();
            mConnected = false;
        }
    }
    /// @brief Sets the callback function.
    void setCallback(
        const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                (const std::string &, const void *, size_t )> &callback)
    {
        mCallback = callback;
    }
    /// @brief Sets the socket options
    virtual
    void setSocketOptions(
        const UMPS::Messaging::RouterDealer::ReplyOptions &options)
    {
         auto zapOptions = options.getZAPOptions();
         auto receiveHighWaterMark = options.getHighWaterMark(); 
         auto sendHighWaterMark = options.getHighWaterMark();
         auto address = options.getAddress();
         // Set the ZAP options
         zapOptions.setSocketOptions(&*mServer);
         // Set the high water mark
         mServer->set(zmq::sockopt::rcvhwm, receiveHighWaterMark);
         mServer->set(zmq::sockopt::sndhwm, sendHighWaterMark);
         if (options.haveRoutingIdentifier())
         {
             mServer->set(zmq::sockopt::routing_id,
                          options.getRoutingIdentifier());
         }
         // Made it this far -> save options
         mOptions = options;
    }
    /// @brief Connect or bind.
    virtual
    void connectOrBind(
        const UMPS::Messaging::RouterDealer::ReplyOptions &options,
        const bool lConnect)
    {
        // Make sure connection is terminated
        disconnect();
        // Update the socket options
        setSocketOptions(options);
        // Set the callback
        setCallback(options.getCallback());
        // Handle IPC
        auto address = mOptions.getAddress();
        ::createIPCDirectoryFromConnectionString(address, &*mLogger);
        // Now connect
        if (lConnect)
        {
            mLogger->debug("Reply attempting to connect to: " + address);
            mServer->connect(address);
            mLogger->debug("Reply connected to: " + address);
        }
        else
        {
            mLogger->debug("Reply attempting to bind to: " + address);
            mServer->connect(address);
            mLogger->debug("Reply bound to: " + address);
        }
        // Resolve end point
        mAddress = address;
        if (address.find("tcp") != std::string::npos ||
            address.find("ipc") != std::string::npos)
        {
            mAddress = mServer->get(zmq::sockopt::last_endpoint);
        }
        // We are now connected
        mConnected = true;
        // Update the socket details
        updateSocketDetails(lConnect);
    }
    /// @brief Connect.  Use this options when connecting to a stable end point.
    virtual
    void connect(const UMPS::Messaging::RouterDealer::ReplyOptions &options)
    {
        constexpr bool lConnect{true};
        connectOrBind(options, lConnect);
    }
    /// @brief Bind.  Use this options when this is the stable end point to
    ///        which others will connect.
    virtual
    void bind(const UMPS::Messaging::RouterDealer::ReplyOptions &options)
    {
        constexpr bool lConnect{false};
        connectOrBind(options, lConnect);
    }
    /// @brief Update socket details.
    virtual void updateSocketDetails(const bool lConnect)
    {
        mSocketDetails.setAddress(mAddress);
        mSocketDetails.setSecurityLevel(
            mOptions.getZAPOptions().getSecurityLevel());
        // This socket is connecting so others would bind
        namespace UCI = UMPS::Services::ConnectionInformation;
        if (lConnect)
        {
            mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::Bind);
        }
        else
        {
            mSocketDetails.setConnectOrBind(UCI::ConnectOrBind::Connect);
        }
    }
    /// @result True indicates the poller is running in a background thread.
    [[nodiscard]] virtual bool isRunning() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mRunning && mConnected;
    }
    /// @result True indicates the reply is connnect.
    virtual bool isConnected() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mConnected;
    }
    /// @brief Toggle poll loop as running / not running.
    virtual void setRunning(const bool running = true)
    {
        std::scoped_lock lock(mMutex);
        mRunning = running;
    }
    /// @brief This performs the polling loop that will:
    ///        1.  Wait for messages from the client.
    ///        2.  Call the callback to process the message.
    ///        3.  Return the result of the callback to the client.
    virtual void poll()
    {
        mLogger->debug("Reply starting poll loop...");
        auto logLevel = mLogger->getLevel();
        while (isRunning())
        {
            // Poll
            std::array<zmq::pollitem_t, 1> pollItems
            {
                 {{mServer->handle(), 0, ZMQ_POLLIN, 0}}
            };
            zmq::poll(pollItems.data(), pollItems.size(), mPollTimeOut.count());
            // Got something
            if (pollItems[0].revents & ZMQ_POLLIN)
            {
                // Get the next message
                zmq::multipart_t messagesReceived(*mServer);
                if (logLevel >= UMPS::Logging::Level::Debug)
                {
                    mLogger->debug("Reply received message!");
                }
#ifndef NDEBUG
                assert(messagesReceived.size() == 2);
#else
                if (messagesReceived.size() != 2)
                {
                    mLogger->error("Only 2-part messages handled");
                    continue;
                }
#endif
                std::string messageType = messagesReceived.at(0).to_string();
                auto messageContents = reinterpret_cast<const void *>
                                       (messagesReceived.at(1).data());
                auto messageSize = messagesReceived.at(1).size();
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
    /// @brief Sends a message header / contents message.
    void send(const std::string &header, const std::string &message)
    {
        if (!isConnected())
        {
            throw std::runtime_error("Reply not connected");
        }
        zmq::const_buffer headerBuffer{header.data(), header.size()};
        mServer->send(headerBuffer, zmq::send_flags::sndmore);
        zmq::const_buffer messageBuffer{message.data(),
                                        message.size()};
        if (mLogger->getLevel() >= UMPS::Logging::Level::Debug)
        {
            mLogger->debug("Reply sending message: " + header);
        }
        mServer->send(messageBuffer);
    }
    /// @brief Sends an IMessage.
    void send(const UMPS::MessageFormats::IMessage &message)
    {
        auto messageType = message.getMessageType();
        if (messageType.empty())
        {
            mLogger->warn("Message type is empty");
        }
        auto messageContents = message.toMessage();
        if (messageContents.empty())
        {
            mLogger->warn("Message is empty");
        }
        // Send the message
        send(messageType, messageContents);
    }
    /// @brief Stops the service.
    void stop()
    {
        setRunning(false);
        if (mPollThread.joinable()){mPollThread.join();}
    }
    /// @brief Starts the service .
    void start()
    {
        // Ensure service isn't running before spinning off a thread.
        stop();
        // Connected?
        if (!isConnected())
        {
            throw std::runtime_error("No connection");
        } 
        // Now start the service.
        setRunning(true);
        mPollThread = std::thread(&ReplySocket::poll, this);
    } 
    /// @brief Return the socket details.
    [[nodiscard]]
    UMPS::Services::ConnectionInformation::SocketDetails::Reply
        getSocketDetails() const
    {
        if (!isConnected()){throw std::runtime_error("Not initialized");}
        return mSocketDetails;
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<zmq::socket_t> mServer{nullptr};
    UMPS::Messaging::RouterDealer::ReplyOptions mOptions;
    std::function<
          std::unique_ptr<UMPS::MessageFormats::IMessage>
          (const std::string &messageType, const void *contents,
           const size_t length)
    > mCallback;
    std::thread mPollThread;
    std::string mAddress;
    std::chrono::milliseconds mPollTimeOut{10};
    UMPS::Services::ConnectionInformation::SocketDetails::Reply mSocketDetails;
    bool mConnected{false};
    bool mRunning{false};
};
}
#endif
