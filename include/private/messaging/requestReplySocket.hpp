#ifndef PRIVATE_MESSAGING_REQUEST_REPLY_SOCKET_HPP
#define PRIVATE_MESSAGING_REQUEST_REPLY_SOCKET_HPP
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
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/messaging/socketOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "private/messaging/ipcDirectory.hpp"
namespace
{
/// @brief This is a base implementation for a request/reply socket.
class RequestReplySocket
{
public:
    /// @brief C'tor
    RequestReplySocket(const zmq::socket_type socketType,
                       std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                       std::shared_ptr<UMPS::Logging::ILog> logger = nullptr) :
        mSocketType(socketType)
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
        if (socketType == zmq::socket_type::rep)
        {
            mSocket = std::make_unique<zmq::socket_t> (*contextPtr,
                                                       zmq::socket_type::rep);
        }
        else if (socketType == zmq::socket_type::req)
        {
            mSocket = std::make_unique<zmq::socket_t> (*contextPtr,
                                                       zmq::socket_type::req);
        }
        else if (socketType == zmq::socket_type::dealer)
        {
            mSocket
                = std::make_unique<zmq::socket_t> (*contextPtr,
                                                   zmq::socket_type::dealer);
        }
        else
        {
            throw std::invalid_argument("Invalid socket type");
        }
    }
    /// @brief Destructor
    virtual ~RequestReplySocket()
    {
        disconnect();
    }
    /// @brief Disconnect
    virtual void disconnect()
    {
        stop(); // Make sure service thread stopped before disconnecting
        if (mConnected)
        {
            mSocket->disconnect(mAddress);
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
        mHaveCallback = true;
    }
    /// @brief Sets the socket options.
    void setSocketOptions(const UMPS::Messaging::SocketOptions &options)
    {
        if (!options.haveAddress())
        {
            throw std::invalid_argument("Address not set");
        }
        auto zapOptions = options.getZAPOptions();
        auto receiveHighWaterMark = options.getReceiveHighWaterMark();
        auto sendHighWaterMark = options.getSendHighWaterMark();
        auto receiveTimeOut = options.getReceiveTimeOut();
        auto sendTimeOut = options.getSendTimeOut();  
        auto lingerPeriod = options.getLingerPeriod(); 
        // Set the socket options
        zapOptions.setSocketOptions(&*mSocket);
        mSocket->set(zmq::sockopt::rcvhwm, receiveHighWaterMark);
        mSocket->set(zmq::sockopt::sndhwm, sendHighWaterMark);
        mSocket->set(zmq::sockopt::rcvtimeo,
                     static_cast<int> (receiveTimeOut.count()));
        mSocket->set(zmq::sockopt::sndtimeo,
                     static_cast<int> (sendTimeOut.count()));
        mSocket->set(zmq::sockopt::linger,
                     static_cast<int> (lingerPeriod.count()));
        if (options.haveRoutingIdentifier())
        {
            mSocket->set(zmq::sockopt::routing_id,
                         options.getRoutingIdentifier());
        }
        // Made it this far -> save options
        mOptions = options;
    }
    /// @brief Connect or bind.
    /// @param[in] options   The socket options.
    /// @param[in] lConnect  When true this will connect.  When false this
    ///                      will bind.  Note, stable end points bind in ZMQ.
    /// @throws std::invalid_argument if necessary parameters are missing.
    /// @throws std::runtime_error if there was a failure to connect or bind.
    void connectOrBind(const UMPS::Messaging::SocketOptions &options,
                       const bool lConnect)
    {
        // Make sure connection is terminated
        disconnect();
        // Update the socket options
        setSocketOptions(options);
        // Set the callback
        if (options.haveCallback())
        {
            setCallback(options.getCallback());
        }
        // Handle IPC
        auto address = mOptions.getAddress();
        ::createIPCDirectoryFromConnectionString(address, &*mLogger);
        // Now connect
        if (lConnect)
        {
            mLogger->debug("Attempting to connect to: " + address);
            mSocket->connect(address);
            mLogger->debug("Connected to: " + address);
        }
        else
        {
            mLogger->debug("Attempting to bind to: " + address);
            mSocket->connect(address);
            mLogger->debug("Bound to: " + address);
        }
        // Resolve end point
        mAddress = address;
        if (address.find("tcp") != std::string::npos ||
            address.find("ipc") != std::string::npos)
        {
            mAddress = mSocket->get(zmq::sockopt::last_endpoint);
        }
        // We are now connected
        mConnected = true;
        // Update the socket details
        updateSocketDetails(lConnect);
    }
    /// @brief Update socket details.
    virtual void updateSocketDetails(const bool lConnect)
    {
        if (mSocketType == zmq::socket_type::req)
        {
            mRequestSocketDetails.setAddress(mAddress);
            mRequestSocketDetails.setSecurityLevel(
                mOptions.getZAPOptions().getSecurityLevel());
            // This socket is connecting so others would bind
            namespace UCI = UMPS::Services::ConnectionInformation;
            if (lConnect)
            {
                mRequestSocketDetails.setConnectOrBind(
                    UCI::ConnectOrBind::Bind);
            }
            else
            {
                mRequestSocketDetails.setConnectOrBind(
                    UCI::ConnectOrBind::Connect);
            }
        }
        else if (mSocketType == zmq::socket_type::rep)
        {
            mReplySocketDetails.setAddress(mAddress);
            mReplySocketDetails.setSecurityLevel(
                mOptions.getZAPOptions().getSecurityLevel());
            // This socket is connecting so others would bind
            namespace UCI = UMPS::Services::ConnectionInformation;
            if (lConnect)
            {
                mReplySocketDetails.setConnectOrBind(
                    UCI::ConnectOrBind::Bind);
            }
            else
            {
                mReplySocketDetails.setConnectOrBind(
                    UCI::ConnectOrBind::Connect);
            }
        }
        else if (mSocketType == zmq::socket_type::dealer)
        {
            mDealerSocketDetails.setAddress(mAddress);
            mDealerSocketDetails.setSecurityLevel(
                mOptions.getZAPOptions().getSecurityLevel());
            // This socket is connecting so others would bind
            namespace UCI = UMPS::Services::ConnectionInformation;
            if (lConnect)
            {
                mDealerSocketDetails.setConnectOrBind(
                    UCI::ConnectOrBind::Bind);
            }
            else
            {
                mDealerSocketDetails.setConnectOrBind(
                    UCI::ConnectOrBind::Connect);
            }
        }
#ifndef NDEBUG
        else
        {
            assert(false);
        }
#endif
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
        if (!mHaveCallback)
        {
            throw std::runtime_error("Callback not set for poll");
        }
        mLogger->debug("Starting poll loop...");
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
                    mLogger->debug("Poller received message!");
                }
                if (messagesReceived.size() != 2)
                {
                    mLogger->error("Unhandled message size: "
                                 + std::to_string(messagesReceived.size())
                                 + ".  Only 2-part messages handled.");
#ifndef NDEBUG
                    assert(messagesReceived.size() == 2);
#endif
                    continue;
                }
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
        mLogger->debug("Poll loop finished");
    }
    /// @brief Receives a message header / contents message.
    /// @param[in] flags  The receive flags.  By default this will block.
    ///                   However, = zmq::recv_flags::dontwait then this
    ///                   will return an empty message if there are no 
    ///                   messages.
    /// @result The received message.  This may be empty.
    [[nodiscard]] zmq::multipart_t receive(
        const zmq::recv_flags flags = zmq::recv_flags::none) const
    {
        // Receive all parts of the message
        zmq::multipart_t responseReceived;
        responseReceived.recv(*mSocket, static_cast<int> (flags));
        if (responseReceived.empty()){return responseReceived;} // Timeout
#ifndef NDEBUG
        assert(responseReceived.size() == 2); 
#else
        if (responseReceived.size() != 2)
        {
            pImpl->mLogger->error("Only 2-part messages handled");
            throw std::runtime_error("Only 2-part messages handled");
        }
#endif
        return responseReceived;
    }
    /// @brief Sends a message header / contents message.
    void send(const std::string &header, const std::string &message)
    {
        if (!isConnected())
        {
            throw std::runtime_error("Reply not connected");
        }
        zmq::const_buffer headerBuffer{header.data(), header.size()};
        mSocket->send(headerBuffer, zmq::send_flags::sndmore);
        zmq::const_buffer messageBuffer{message.data(),
                                        message.size()};
        if (mLogger->getLevel() >= UMPS::Logging::Level::Debug)
        {
            mLogger->debug("Reply sending message: " + header);
        }
        mSocket->send(messageBuffer);
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
        mPollThread = std::thread(&RequestReplySocket::poll, this);
    } 
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<zmq::socket_t> mSocket{nullptr};
    std::function<
          std::unique_ptr<UMPS::MessageFormats::IMessage>
          (const std::string &messageType, const void *contents,
           const size_t length)
    > mCallback;
    UMPS::Messaging::SocketOptions mOptions;
    UMPS::Messaging::RouterDealer::RequestOptions mRequestOptions;
    UMPS::Messaging::RouterDealer::ReplyOptions mReplyOptions;
    UMPS::Services::ConnectionInformation::SocketDetails::Dealer
        mDealerSocketDetails;
    UMPS::Services::ConnectionInformation::SocketDetails::Reply
        mReplySocketDetails;
    UMPS::Services::ConnectionInformation::SocketDetails::Request
        mRequestSocketDetails;
    std::thread mPollThread;
    std::string mAddress;
    zmq::socket_type mSocketType;
    std::chrono::milliseconds mPollTimeOut{10};
    bool mConnected{false};
    bool mRunning{false};
    bool mHaveCallback{false};
};
///--------------------------------------------------------------------------///
///                                Request Socket                            ///
///--------------------------------------------------------------------------///
/// @brief A request socket.                                                
class RequestSocket : public RequestReplySocket 
{
public:
    /// C'tor
    RequestSocket(std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                  std::shared_ptr<UMPS::Logging::ILog> logger = nullptr) :
       RequestReplySocket(zmq::socket_type::req, context, logger)
    {
    }
    /// @brief Converts the socket options
    [[nodiscard]]
    UMPS::Messaging::SocketOptions
        convertSocketOptions(
           const UMPS::Messaging::RouterDealer::RequestOptions &options)
    {
        // Convert
        UMPS::Messaging::SocketOptions socketOptions;
        socketOptions.setAddress(options.getAddress());
        socketOptions.setZAPOptions(options.getZAPOptions());
        socketOptions.setReceiveHighWaterMark(options.getHighWaterMark());
        socketOptions.setSendHighWaterMark(options.getHighWaterMark());
        // Now set the socket options
        setSocketOptions(socketOptions); 
    }
    /// @brief Connect
    void connect(const UMPS::Messaging::RouterDealer::RequestOptions &options)
    {
        auto socketOptions = convertSocketOptions(options); // Throws
        constexpr bool lConnect{true};
        connectOrBind(socketOptions, lConnect); 
    }
    /// @brief Bind
    void bind(const UMPS::Messaging::RouterDealer::RequestOptions &options)
    {
        auto socketOptions = convertSocketOptions(options); // Throws
        constexpr bool lConnect{false};
        connectOrBind(socketOptions, lConnect); 
    }
    /// Socket details
    [[nodiscard]]
    UMPS::Services::ConnectionInformation::SocketDetails::Request
        getSocketDetails() const
    {
        if (!isConnected()){throw std::runtime_error("Not initialized");}
        return mRequestSocketDetails;
    }
///private:
};
///--------------------------------------------------------------------------///
///                                Reply Socket                              ///
///--------------------------------------------------------------------------///
/// @brief A reply socket.
class ReplySocket : public RequestReplySocket 
{
public:
    /// C'tor
    ReplySocket(std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                std::shared_ptr<UMPS::Logging::ILog> logger = nullptr) :
       RequestReplySocket(zmq::socket_type::rep, context, logger)
    {
    }
    /// @brief Converts the socket options
    [[nodiscard]]
    UMPS::Messaging::SocketOptions
        convertSocketOptions(
           const UMPS::Messaging::RouterDealer::ReplyOptions &options)
    {
        // Convert
        UMPS::Messaging::SocketOptions socketOptions;
        socketOptions.setAddress(options.getAddress());
        socketOptions.setZAPOptions(options.getZAPOptions());
        socketOptions.setReceiveHighWaterMark(options.getHighWaterMark());
        socketOptions.setSendHighWaterMark(options.getHighWaterMark());
        socketOptions.setCallback(options.getCallback()); // Need this
        // Now set the socket options
        setSocketOptions(socketOptions);
    }
    /// @brief Connect
    void connect(const UMPS::Messaging::RouterDealer::ReplyOptions &options)
    {
        auto socketOptions = convertSocketOptions(options); // Throws
        constexpr bool lConnect{true};
        connectOrBind(socketOptions, lConnect); 
    }
    /// @brief Bind
    void bind(const UMPS::Messaging::RouterDealer::ReplyOptions &options)
    {
        auto socketOptions = convertSocketOptions(options); // Throws
        constexpr bool lConnect{false};
        connectOrBind(socketOptions, lConnect);
    }
    /// Socket details
    [[nodiscard]]
    UMPS::Services::ConnectionInformation::SocketDetails::Reply
        getSocketDetails() const
    {
        if (!isConnected()){throw std::runtime_error("Not initialized");}
        return mReplySocketDetails;
    }
///private:
};
}
#endif
