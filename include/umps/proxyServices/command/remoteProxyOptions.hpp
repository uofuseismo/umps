#ifndef UMPS_PROXY_SERVICES_COMMAND_REMOTE_PROXY_OPTIONS_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REMOTE_PROXY_OPTIONS_HPP
#include <memory>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class RemoteProxyOptions "remoteProxyOptions.hpp" "umps/proxyServices/command/remoteProxyOptions.hpp"
/// @brief Defines the options for the command proxy.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RemoteProxyOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RemoteProxyOptions();
    /// @brief Copy constructor.
    RemoteProxyOptions(const RemoteProxyOptions &options);
    /// @brief Move constructor.
    RemoteProxyOptions(RemoteProxyOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options.
    RemoteProxyOptions& operator=(const RemoteProxyOptions &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    RemoteProxyOptions& operator=(RemoteProxyOptions &&options) noexcept;
    /// @}

    /// @name Router Options
    /// @{

    /// @brief Sets the address to which clients connect.
    /// @throws std::invalid_argument if the address is empty.
    void setFrontendAddress(const std::string &address);
    /// @result The frontend address.
    /// @throws std::invalid_argument if \c haveFrontendAddress is false.
    [[nodiscard]] std::string getFrontendAddress() const;
    /// @result True indicates the frontend address was set.
    [[nodiscard]] bool haveFrontendAddress() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued by the frontend.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."
    /// @throws std::invalid_argument if the high water mark is negative.
    void setFrontendHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getFrontendHighWaterMark() const noexcept;
    /// @}

    /// @name Dealer Options
    /// @{

    /// @brief Sets the address to which servers connect.
    /// @param[in] address  The address to which servers will connect.
    /// @throws std::invalid_argument if the address is empty.
    void setBackendAddress(const std::string &address);
    /// @result The backend address.
    /// @throws std::invalid_argument if \c haveBackendAddress is false.
    [[nodiscard]] std::string getBackendAddress() const;
    /// @result True indicates the backend address was set.
    [[nodiscard]] bool haveBackendAddress() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued by the backend.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."
    /// @throws std::invalid_argument if the high water mark is negative.
    void setBackendHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getBackendHighWaterMark() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol Options
    /// @{

    /// @brief Defines the ZAP options to be used when configuring the socket.
    /// @param[in] options  The ZAP options.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.  By default this will configure sockets with
    ///         the grasslands (no security) pattern.
    [[nodiscard]] Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~RemoteProxyOptions();
    /// @}
private:
    class RemoteProxyOptionsImpl;
    std::unique_ptr<RemoteProxyOptionsImpl> pImpl;
};
}
#endif
