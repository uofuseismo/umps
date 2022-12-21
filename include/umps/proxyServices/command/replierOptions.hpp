#ifndef UMPS_PROXY_SERVICES_COMMAND_REPLIER_OPTIONS_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REPLIER_OPTIONS_HPP
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
 namespace ProxyServices::Command
 {
  class ModuleDetails;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class ReplierOptions "replierOptions.hpp" "umps/proxyServices/command/replierOptions.hpp"
/// @brief Defines the options for the thread managing remote activity to
///        respond.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_uRemoteCommand
class ReplierOptions
{
public:
    /// @name Constructor
    /// @{

    /// @brief Constructor.
    ReplierOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    ReplierOptions(const ReplierOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    ReplierOptions(ReplierOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the options.
    ReplierOptions& operator=(const ReplierOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    ReplierOptions& operator=(ReplierOptions &&options) noexcept;
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
    /// @}

    /// @name Module Details (Required)
    /// @{

    /// @brief Sets the module details.
    /// @param[in] details  The module details.
    /// @throws std::invalid_argument if details.haveName() is false.
    void setModuleDetails(const ModuleDetails &details);
    /// @result The module details.
    /// @throws std::runtime_error if \c haveModuleDetails() is false.
    [[nodiscard]] ModuleDetails getModuleDetails() const; 
    /// @result True indicates the module details were set.
    [[nodiscard]] bool haveModuleDetails() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol
    /// @{

    /// @brief Sets the ZAP options.
    /// @param[in] options  The ZAP options which will define the socket's
    ///                     security protocol.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @name High Water Mark
    /// @{

    /// @brief Influences the maximum number of request messages to cache on the
    ///        socket.
    /// @param[in] highWaterMark  The approximate max number of request messages
    ///                           to cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setReceiveHighWaterMark(int highWaterMark);
    /// @result The approximate number of request messages to cache.
    [[nodiscard]] int getReceiveHighWaterMark() const noexcept;
    /// @brief Influences the maximum number of reply messages to cache on the
    ///        socket.
    /// @param[in] highWaterMark  The approximate max number of reply messages
    ///                           to cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setSendHighWaterMark(int highWaterMark);
    /// @result The approximate number of response messages to cache.
    [[nodiscard]] int getSendHighWaterMark() const noexcept;
    /// @}

    /// @name Polling Interval
    /// @{

    /// @brief Sets the polling interval for the process that communicates with
    ///        the module registry.
    /// @param[in] pollingInterval  The polling timeout.  After this time the
    ///                             module registry polling process can check
    ///                             other things, like if the program is
    ///                             shutting down.  This must be a balance
    ///                             between being too short which unnecessarily
    ///                             chews cycles and being too long so as to
    ///                             make this utility unresponsive.
    /// @throws std::invalid_argument if the polling interval is negative.
    void setPollingTimeOut(const std::chrono::milliseconds &pollingInterval);
    /// @result The polling interval.
    [[nodiscard]] std::chrono::milliseconds getPollingTimeOut() const noexcept;
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
    ~ReplierOptions();
    /// @}
private:
    class ReplierOptionsImpl;
    std::unique_ptr<ReplierOptionsImpl> pImpl;
};
}
#endif
