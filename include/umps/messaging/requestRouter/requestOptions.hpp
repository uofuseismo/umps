#ifndef UMPS_MESSAGING_REQUESTROUTER_REQUESTOPTIONS_HPP
#define UMPS_MESSAGING_REQUESTROUTER_REQUESTOPTIONS_HPP
#include <memory>
#include <chrono>
#include "umps/authentication/enums.hpp"
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
namespace UMPS::Messaging::RequestRouter
{
/// @class RequestOptions "requestOptions.hpp" "umps/messaging/requestRouter/requestOptions.hpp"
/// @brief Defines the request socket options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RequestRouter
class RequestOptions
{
public:
    /// @name Constructor
    /// @{

    /// @brief Constructor.
    RequestOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    RequestOptions(const RequestOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    RequestOptions(RequestOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of options.
    RequestOptions& operator=(const RequestOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    RequestOptions& operator=(RequestOptions &&options) noexcept;
    /// @}

    /// @name Address
    /// @{

    /// @param[in] address  The address to which to connect.
    ///                     For example, "tcp://127.0.0.1:5555".
    /// @throws std::invalid_argument if address is blank.
    void setAddress(const std::string &address);
    /// @result The address to which to connect.
    /// @throws std::runtime_error if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
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
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    /// @result The ZAP options.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;
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

    /// @name Time Out
    /// @{

    /// @brief If the requests waits the timeOut length of time before
    ///        receiving a response then it will return without a message.
    ///        This is useful when the requestor thread has other
    ///        responsibilities.
    /// @param[in] timeOut   The time out duration in milliseconds.  If this is
    ///                      zero then the requestor will immediately return.
    ///                      If this is negative then the requestor will wait
    ///                      indefinitely until a message is received.
    void setTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The time out duration in milliseconds.
    [[nodiscard]] std::chrono::milliseconds getTimeOut() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RequestOptions();
    /// @}
private:
    class RequestOptionsImpl;
    std::unique_ptr<RequestOptionsImpl> pImpl;
};
}
#endif
