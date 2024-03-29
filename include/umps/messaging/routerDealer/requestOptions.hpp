#ifndef UMPS_MESSAGING_ROUTER_DEALER_REQUEST_OPTIONS_HPP
#define UMPS_MESSAGING_ROUTER_DEALER_REQUEST_OPTIONS_HPP
#include <memory>
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
/// @class RequestOptions "requestOptions.hpp" "umps/messaging/routerDealer/requestOptions.hpp"
/// @brief Defines the request socket options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RouterDealer
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

    /// @name Address (Required)
    /// @{

    /// @brief Sets the address of the router to which requests will be
    ///        submitted and from which replies will be received.
    /// @param[in] address  The address of the router.
    /// @throws std::invalid_argument if address is blank.
    void setAddress(const std::string &address);
    /// @result The address to of the router.
    /// @throws std::runtime_error if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
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

    /// @name Message Types
    /// @{

    /// @brief Sets the message types to which to subscriber.
    /// @param[in] messageFormats  The message formats that the requestor will
    ///                            be able to read and deserialize.
    /// @note The requestor has to deserialize responses so these
    ///       should be reply message formats. 
    /// @throws std::invalid_argument if messageFormats.empty() is true.
    void setMessageFormats(const UMPS::MessageFormats::Messages &messageFormats);
    /// @result The message types to which to subscriber.
    /// @throws std::runtime_error if \c haveMessageTypes() is false.
    [[nodiscard]] UMPS::MessageFormats::Messages getMessageFormats() const;
    /// @result True indicates the message types have been set.
    [[nodiscard]] bool haveMessageFormats() const noexcept;
    /// @}

    /// @name High Water Mark
    /// @{

    /// @param[in] highWaterMark  The approximate max number of request messages
    ///                           to cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setSendHighWaterMark(int highWaterMark);
    /// @result The send high water mark.  The default is 0.
    [[nodiscard]] int getSendHighWaterMark() const noexcept;
    /// @param[in] highWaterMark  The approximate max number of response messages
    ///                           to cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setReceiveHighWaterMark(int highWaterMark);
    /// @result The send high water mark.  The default is 0.
    [[nodiscard]] int getReceiveHighWaterMark() const noexcept;
    /// @}

    /// @name Time Out
    /// @{

    /// @param[in] timeOut  The time to wait before a request send times out.
    ///                     If this is negative then the wait time will be
    ///                     infinite.
    void setSendTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The send time out.  The default is 0 (return immediately).
    [[nodiscard]] std::chrono::milliseconds getSendTimeOut() const noexcept;
    /// @param[in] timeOut  The time to wait before a response from the server
    ///                     result in a time out.  If this is negative then the
    ///                     wait time will be infinite.
    void setReceiveTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The send time out.  The default is -1 (infinite).
    [[nodiscard]] std::chrono::milliseconds getReceiveTimeOut() const noexcept;
    /// @}

    /// @name Time Out
    /// @{

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
