#ifndef UMPS_MESSAGING_ROUTER_DEALER_REPLY_OPTIONS_HPP
#define UMPS_MESSAGING_ROUTER_DEALER_REPLY_OPTIONS_HPP
#include <memory>
#include <functional>
#include <chrono>
// Forward declarations
namespace UMPS
{
 namespace MessageFormats
 {
  class Messages;
  class IMessage;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::Messaging::RouterDealer
{
/// @class ReplyOptions "requestOptions.hpp" "umps/messaging/routerDealer/replyOptions.hpp"
/// @brief Defines the reply socket options.
/// @details After receiving a message from the dealer the router will use the
///          callback function defined in this class to reply to the request.
///          It is important that the callback not throw or fail.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RouterDealer
class ReplyOptions
{
public:
    /// @name Constructor
    /// @{

    /// @brief Constructor.
    ReplyOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    ReplyOptions(const ReplyOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    ReplyOptions(ReplyOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of options.
    ReplyOptions& operator=(const ReplyOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    ReplyOptions& operator=(ReplyOptions &&options) noexcept;
    /// @}

    /// @name Address (Required)
    /// @{

    /// @brief The address of the dealer to which the service will connect.
    /// @param[in] address  The address to from which requests will be
    ///                     received and to which replies will be sent.
    /// @throws std::invalid_argument if address is blank.
    void setAddress(const std::string &address);
    /// @result The address to of the dealer.
    /// @throws std::runtime_error if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name Callback (Required)
    /// @{

    /// @brief The callback function is specified by the user to process
    ///        messages.
    /// @details Your application will create a specific request/reply pair.
    ///          The router/dealer will forward request messages to the
    ///          reply service.  Nominally, the callback will validate the
    ///          message type and, based on the type, appropriately 
    ///          convert the message payload to an appropriate request 
    ///          IMessage.  The request will then be processed and responded
    ///          to with a corresponding response IMessage that the requestor
    ///          can handle.
    /// @param[in] callback  The callback function which processes the message.
    /// @note It is important this function never throw, crash, etc.  This is
    ///       when exiting gracefully matters.
    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                         (const std::string &messageType, const void *data, size_t length)> &callback);
    /// @result The callback function.
    /// @throws std::invalid_argument if \c haveCallback() is false.
    [[nodiscard]]
    std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *data, size_t length)> getCallback() const;
    /// @result True indicates the callback was set.
    [[nodiscard]] bool haveCallback() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol
    /// @{

    /// @brief Sets the ZAP options.
    /// @param[in] options  The ZAP options which will define the socket's
    ///                     security protocol.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.
    [[nodiscard]] Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @name Time Out
    /// @{

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

    /// @name High Water Mark
    /// @{

    /// @param[in] highWaterMark  The approximate max number of request messages
    ///                           to cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setSendHighWaterMark(int highWaterMark);
    /// @result The request high water mark.  The default is 0.
    [[nodiscard]] int getSendHighWaterMark() const noexcept;
    /// @param[in] highWaterMark  The approximate max number of response messages
    ///                           to cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setReceiveHighWaterMark(int highWaterMark);
    /// @result The response high water mark.  The default is 0.
    [[nodiscard]] int getReceiveHighWaterMark() const noexcept;
    /// @}

    /// @name Routing Identifier
    /// @{

    /// @brief This provides a way for a reply socket to identify itself for the
    ///        benefit of the proxy that must route messages to specific 
    ///        endpoints.  For most applications this is not required.
    /// @param[in] identifier  The reply socket identifier.  If
    ///                        identifier.size() exceeds 255 then it will be
    ///                        truncated.
    void setRoutingIdentifier(const std::string &identifier);
    /// @result The routing identifier.
    /// @throws std::runtime_error if \c haveRoutingIdentifier() is false.
    [[nodiscard]] std::string getRoutingIdentifier() const;
    /// @result True indicates the routing identifier was set.
    [[nodiscard]] bool haveRoutingIdentifier() const noexcept;
    /// @}

    /// @name Message types
    /// @{

    /// @brief Adds a message format that the request can receive from the
    ///        ZeroMQ router.
    /// @param[in] message  The message type.
    //void addMessageFormat(std::unique_ptr<UMPS::MessageFormats::IMessage> &message);
    /// @result The types of messages that the router can receive via
    ///         ZeroMQ.
    /// @note If this is empty then the request will listen for all messages.
    //[[nodiscard]] UMPS::MessageFormats::Messages getMessageFormats() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~ReplyOptions();
    /// @}
private:
    class ReplyOptionsImpl;
    std::unique_ptr<ReplyOptionsImpl> pImpl;
};
}
#endif
