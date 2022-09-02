#ifndef UMPS_MESSAGING_REQUESTROUTER_ROUTEROPTIONS_HPP
#define UMPS_MESSAGING_REQUESTROUTER_ROUTEROPTIONS_HPP
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
/// @ingroup MessagingPatterns_ReqRep_RequestRouter
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

    /// @name Address (Required)
    /// @{

    /// @brief The address of the router to which the clients will connect.
    /// @param[in] address  The address to which requests will be
    ///                     received and from which replies will be sent.
    /// @throws std::invalid_argument if address is blank.
    void setAddress(const std::string &address);
    /// @result The address to of the router.
    /// @throws std::runtime_error if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name Callback (Required)
    /// @{

    /// @brief The callback function that defines how the router will reply to
    ///        a request message. 
    /// @details Your application will create a specific request/router pair.
    ///          Nominally, the callback will validate the
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
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    /// @result The ZAP options.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;

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
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RouterOptions();
    /// @}
private:
    class RouterOptionsImpl;
    std::unique_ptr<RouterOptionsImpl> pImpl;
};
}
#endif
