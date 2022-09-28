#ifndef UMPS_SERVICES_COMMAND_REMOTEREPLIEROPTIONS_HPP
#define UMPS_SERVICES_COMMAND_REMOTEREPLIEROPTIONS_HPP
#include <memory>
#include <chrono>
#include <functional>
// Forward declarations
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Messaging::RouterDealer
 {
  class ReplyOptions;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::Services::Command
{
/// @class RemoteReplierOptions "remoteReplierOptions.hpp" "umps/command/remoteReplierOptions.hpp"
/// @brief Defines the options for the thread managing remote activity to
///        respond.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RemoteReplierOptions
{
public:
    /// @name Constructor
    /// @{

    /// @brief Constructor.
    RemoteReplierOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    RemoteReplierOptions(const RemoteReplierOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    RemoteReplierOptions(RemoteReplierOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the options.
    RemoteReplierOptions& operator=(const RemoteReplierOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    RemoteReplierOptions& operator=(RemoteReplierOptions &&options) noexcept;
    /// @}

    /// @name Address (Required)
    /// @{

    /// @brief Sets the address of the dealer from which request will be
    ///        received and to which replies will be sent.
    /// @param[in] address  The address of the dealer to which to connect.
    /// @throws std::invalid_argument if address is blank.
    void setAddress(const std::string &address);
    /// @result True indicates that the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name Callback (Required)
    /// @{

    /// @brief The callback function that processes requests.
    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                         (const std::string &messageType, const void *data, size_t length)> &callback);
    /// @result True indicates the callback was set.
    [[nodiscard]] bool haveCallback() const noexcept;
    /// }

    /// @name High Water Mark
    /// @{

    /// @brief Influences the maximum number of messages to cache on the socket.
    /// @param[in] highWaterMark  The approximate max number of messages to 
    ///                           cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setHighWaterMark(int highWaterMark);
    /// @}

    /// @name ZeroMQ Authentication Protocol
    /// @{

    /// @brief Sets the ZAP options.
    /// @param[in] options  The ZAP options which will define the socket's
    ///                     security protocol.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @}

    /// @result The reply options.
    /// @throws std::runtime_error if \c haveAddress() or \c haveCallback()
    ///         is false.
    [[nodiscard]] UMPS::Messaging::RouterDealer::ReplyOptions getOptions() const;

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RemoteReplierOptions();
    /// @}
private:
    class RemoteReplierOptionsImpl;
    std::unique_ptr<RemoteReplierOptionsImpl> pImpl;
};
}
#endif
