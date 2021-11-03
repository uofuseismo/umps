#ifndef UMPS_MESSAGING_REQUESTROUTER_ROUTEROPTIONS_HPP
#define UMPS_MESSAGING_REQUESTROUTER_ROUTEROPTIONS_HPP
#include <map>
#include <memory>
#include <functional>
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
/// @class RouterOptions "routerOptions.hpp" "umps/messaging/requestRouter/routerOptions.hpp"
/// @brief Defines the router socket options.  A router works like a service.
///        After binding to the endpoint, the service will run in a thread
///        according to the following pseudo-code
///        while (true)
///        {
///            Wait for message on the end point until specified time out
///            if (Message Received)
///            {
///                Process message with callback
///                Return response to sender.
///            }
///            if (Termination Requested)
///            {
///                Exit while loop.
///            }
///        }
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RouterOptions
{
public:
    /// @name Constructor
    /// @{
    /// @brief Constructor.
    RouterOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    RouterOptions(const RouterOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    RouterOptions(RouterOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of options.
    RouterOptions& operator=(const RouterOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    RouterOptions& operator=(RouterOptions &&options) noexcept;
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

    /// @name Poll Time Out
    /// @{
    /// @brief When the service starts a thread will monitor messages on the
    ///        endpoint by "polling."  This controls how long the thread will
    ///        wait.  
    /// @param[in] timeOutInMilliSeconds  The timeout in milliseconds.  If this
    ///                                   is negative then the thread will wait
    ///                                   forever for a message (dangerous).
    ///                                   If this is 0 then thread will return
    ///                                   immediately (this is a good way to get
    ///                                   a thread to report as 100% in use in
    ///                                   a system monitor).
    void setPollTimeOut(const std::chrono::milliseconds &timeOutInMilliSeconds) noexcept;
    /// @result The amount of time to wait before timing out in the polling
    ///         operation.
    [[nodiscard]] std::chrono::milliseconds getPollTimeOut() const noexcept;
    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RouterOptions();
    /// @}

    /// @name Callback
    /// @{
    /// @brief The callback function is specified by the user and processes
    ///        messages of type messageType, with data of type void * which
    ///        the callback converts to the appropriate datatype, and having
    ///        length elements.  The result is an UMPS message that the router
    ///        can transmit on the wire.  The callback function should be very 
    ///        robust and not throw exceptions.
    /// @param[in] callback  The callback function which processes the message.
    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                         (const std::string &messageType, const void *data, size_t length)> &callback);
    /// @result The callback function.
    /// @throws std::invalid_argument if \c haveCallback() is false.
    std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *data, size_t length)> getCallback() const;
    /// @result True indicates the callback was set.
    [[nodiscard]] bool haveCallback() const noexcept; 
    /// @}

    /// @name Message types
    /// @{
    /// @brief Adds a message format that the router can receive from ZeroMQ
    ///        and process.
    /// @param[in] message  The message type.
    void addMessageFormat(std::unique_ptr<UMPS::MessageFormats::IMessage> &message);
    /// @result The types of messages that the router can receive via
    ///         ZeroMQ and process.
    /// @note If this is empty then the router will listen for all messages.
    UMPS::MessageFormats::Messages getMessageFormats() const noexcept;
    /// @}
private:
    class RouterOptionsImpl;
    std::unique_ptr<RouterOptionsImpl> pImpl;
};
}
#endif
