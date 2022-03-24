#ifndef UMPS_PROXYSERVICES_INCREMENTER_REPLIEROPTIONS_HPP
#define UMPS_PROXYSERVICES_INCREMENTER_REPLIEROPTIONS_HPP
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
 namespace Messaging::RouterDealer
 {
  class ReplyOptions;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::ProxyServices::Incrementer
{
/// @class ReplierOptions "replierOptions.hpp" "umps/proxyServices/packetCache/replierOptions.hpp"
/// @brief Defines the options for the incremeter's replier mechanism.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
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
    /// @result A deep copy of options.
    ReplierOptions& operator=(const ReplierOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
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
    /// @result The address to which to connect this service.
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
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.
    [[nodiscard]] Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @result The reply options.
    [[nodiscard]] UMPS::Messaging::RouterDealer::ReplyOptions getReplyOptions() const noexcept;

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
