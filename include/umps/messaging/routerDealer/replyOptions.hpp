#ifndef UMPS_MESSAGING_ROUTERDEALER_REPLYOPTIONS_HPP
#define UMPS_MESSAGING_ROUTERDEALER_REPLYOPTIONS_HPP
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

    /// @name Endpoint (Required)
    /// @{

    /// @param[in] endPoint  The endpoint on which to accept connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @throws std::invalid_argument if endPoint is blank.
    void setEndPoint(const std::string &endPoint);
    /// @result The endpoint to which clients will connect.
    /// @throws std::runtime_error if \c haveEndPoint() is false.
    [[nodiscard]] std::string getEndPoint() const;
    /// @result True indicates that the end point was set.
    [[nodiscard]] bool haveEndPoint() const noexcept;

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

    /// @name High Water Mark
    /// @{

    /// @param[in] highWaterMark  The approximate max number of messages to 
    ///                           cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0.
    [[nodiscard]] int getHighWaterMark() const noexcept;

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

    /// @name Message types
    /// @{

    /// @brief Adds a message format that the request can receive from the
    ///        ZeroMQ router.
    /// @param[in] message  The message type.
    void addMessageFormat(std::unique_ptr<UMPS::MessageFormats::IMessage> &message);
    /// @result The types of messages that the router can receive via
    ///         ZeroMQ.
    /// @note If this is empty then the request will listen for all messages.
    [[nodiscard]] UMPS::MessageFormats::Messages getMessageFormats() const noexcept;
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