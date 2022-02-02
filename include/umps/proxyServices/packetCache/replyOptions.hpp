#ifndef UMPS_PROXYSERVICES_PACKETCACHE_REPLYOPTIONS_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_REPLYOPTIONS_HPP
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
 namespace Messaging::RouterDealer
 {
  class ReplyOptions;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class ReplyOptions "requestOptions.hpp" "umps/proxyServices/packetCache/replyOptions.hpp"
/// @brief Defines the reply socket options for the packet cache.
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
    ~ReplyOptions();
    /// @}
private:
    class ReplyOptionsImpl;
    std::unique_ptr<ReplyOptionsImpl> pImpl;
};
}
#endif
