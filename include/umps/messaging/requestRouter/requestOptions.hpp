#ifndef UMPS_MESSAGING_REQUESTROUTER_REQUESTOPTIONS_HPP
#define UMPS_MESSAGING_REQUESTROUTER_REQUESTOPTIONS_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace MessageFormats
 {
  class Messages;
  class IMessage;
 }
 namespace Messaging::Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::Messaging::RequestRouter
{
/// @class RequestOptions "requestOptions.hpp" "umps/messaging/requestRouter/requestOptions.hpp"
/// @brief Defines the request socket options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
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

    /// @name End Point
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

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RequestOptions();
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
private:
    class RequestOptionsImpl;
    std::unique_ptr<RequestOptionsImpl> pImpl;
};
}
#endif
