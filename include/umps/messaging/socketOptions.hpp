#ifndef UMPS_MESSAGING_SOCKET_OPTIONS_HPP
#define UMPS_MESSAGING_SOCKET_OPTIONS_HPP
#include <memory>
#include <chrono>
#include <functional>
#include <string>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
 namespace MessageFormats
 {
  class IMessage;
  class Messages;
 }
}
namespace UMPS::Messaging
{
/// @class SocketOptions "socketOptions.hpp" "umps/messaging/socketOptions.hpp"
/// @brief This is a base class for setting socket options.
/// @note Not all options are implemented in all socket types.  
///       It is recommended that you use the the appropriate socket's
///       details instead of this class.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class SocketOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    SocketOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    SocketOptions(const SocketOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit options's behavior is
    ///                         undefined.
    SocketOptions(SocketOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @result A deep copy of the input options.
    SocketOptions& operator=(const SocketOptions &options);
    
    /// @result The memory from options moved to this.
    SocketOptions& operator=(SocketOptions &&options) noexcept;
    /// @}

    /// @name Address (Required)
    /// @{

    /// @brief Sets the address to which to bind or connect the socket.
    /// @param[in] address  The address to which to bind or connect this socket.
    /// @throws std::invalid_argument if the protocol is not supported or the
    ///         address is empty.
    void setAddress(const std::string &address);
    /// @result The address to which to bind or connect this socket.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol
    /// @{

    /// @brief Sets the ZAP options.
    /// @param[in] options  The ZAP options which will define the socket's
    ///                     security protocol.
    void setZAPOptions(const Authentication::ZAPOptions &options) noexcept;
    /// @result The ZAP options.
    [[nodiscard]] Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @name Callback
    /// @{

    /// @brief In the generic request-reply pattern a socket will take action
    ///        after a message is received.  In this case, the received message
    ///        will be processed and responded to by this function.
    /// @param[in] callback  The callback function which processes the message.
    /// @note It is important this function never throw, crash, etc.  This is
    ///       when exiting gracefully matters.
    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                         (const std::string &messageType, const void *data, size_t length)> &callback);
    /// @result The callback function.
    /// @throws std::runtime_error if \c haveCallback() is false.
    [[nodiscard]]
    std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *data, size_t length)> getCallback() const;
    /// @result True indicates the callback was set.
    [[nodiscard]] bool haveCallback() const noexcept;
    /// @}

    /// @name Message Formats
    /// @{

    /// @brief Defines the messages formats that will be converted from ZMQ
    ///        messages to IMessages.  This is only necessary for sockets
    ///        that intend to receive messages and are not using a callback
    ///        function to process those messages.
    /// @param[in] messageFormats  The message formats to unpack.
    /// @throws std::invalid_argument if messageFormats is empty.
    void setMessageFormats(const UMPS::MessageFormats::Messages &messageFormats);
    /// @result The message formats that can be deserialized into a class.
    /// @throws std::runtime_error if \c haveMessageFormats() is false.
    [[nodiscard]] UMPS::MessageFormats::Messages getMessageFormats() const;
    /// @result True indicates that message formats were set.
    [[nodiscard]] bool haveMessageFormats() const noexcept;


    /// @name Time Out
    /// @{

    /// @brief If this amount of time ellapses before a message is received then
    ///        the receive message call will return without message.
    ///        This is useful when the receiving thread has other
    ///        responsibilities.
    /// @param[in] timeOut  The receive time out duration in milliseconds.  If
    ///                     this is zero then the requestor will immediately
    ///                     return.  If this is negative then the requestor will
    ///                     wait indefinitely until a message is received.
    void setReceiveTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The time out duration in milliseconds.
    [[nodiscard]] std::chrono::milliseconds getReceiveTimeOut() const noexcept;

    /// @brief If this amount of time ellapses before a message is sent then
    ///        the send message call will return without sending the message.
    ///        This is useful when the requestor thread has other
    ///        responsibilities and the application has, potentially, lost
    ///        connectivity.
    /// @param[in] timeOut  The send time out duration in milliseconds.  If
    ///                     this is zero then the sender will immediately
    ///                     return.  If this is negative then the sender will
    ///                     wait indefinitely until the message is sent.
    void setSendTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The time out duration in milliseconds.
    [[nodiscard]] std::chrono::milliseconds getSendTimeOut() const noexcept;

    /// @brief A polling thread is contained in a while loop that runs until
    ///        the service is terminated.  The polling thread will wait this 
    ///        amount of time for a request before proceeding.
    /// @param[in] timeOut  The amount of time for a polling thread will wait
    ///                     for a message before proceeding.
    /// @throws std::invalid_argument if this is negative.
    void setPollingTimeOut(const std::chrono::milliseconds &timeOut);
    /// @result The amount of time a polling thread will wait before proceeding
    ///         to other activities.
    [[nodiscard]] std::chrono::milliseconds getPollingTimeOut() const noexcept; 
    /// @}

    /// @name Routing Identifier
    /// @{

    /// @brief This provides a way for a socket to identify itself for the
    ///        benefit of a proxy that must route messages to specific 
    ///        endpoints.  For most applications this is not required and
    ///        you should let ZeroMQ automatically populate this information.
    /// @param[in] identifier  The socket identifier.  If identifier.size()
    ///                        exceeds 255 then it will be truncated.
    void setRoutingIdentifier(const std::string &identifier);
    /// @result The routing identifier.
    /// @throws std::runtime_error if \c haveRoutingIdentifier() is false.
    [[nodiscard]] std::string getRoutingIdentifier() const;
    /// @result True indicates the routing identifier was set.
    [[nodiscard]] bool haveRoutingIdentifier() const noexcept;
    /// @}

    /// @name High Water Mark
    /// @{

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued for sending.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setSendHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getSendHighWaterMark() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued for receiving.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setReceiveHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getReceiveHighWaterMark() const noexcept;
    /// @}

    /// @name Linger Period 
    /// {

    /// @brief Defines the behavior of a socket when the socket is closed and
    ///        there still exist unsent messages.
    /// @param[in] period  The linger period.  If this is negative then the
    ///                    pending messages will not be discarded after the
    ///                    socket is closed.  If this is 0 then unsent messages
    ///                    will be immediately discarded when the socket is
    ///                    closed.  If this is positive then unsent messages
    ///                    possibly will be sent during this time.  However,
    ///                    if this time is reached then unsent messages will
    ///                    be discarded. 
    void setLingerPeriod(const std::chrono::milliseconds &period) noexcept;
    /// @result The linger period.  By default this is -1 (infinite).
    [[nodiscard]] std::chrono::milliseconds getLingerPeriod() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~SocketOptions();
    /// @}
private:
    class SocketOptionsImpl;
    std::unique_ptr<SocketOptionsImpl> pImpl;
};
}
#endif
