#ifndef UMPS_PROXY_SERVICES_COMMAND_PROXY_OPTIONS_HPP
#define UMPS_PROXY_SERVICES_COMMAND_PROXY_OPTIONS_HPP
#include <memory>
#include <vector>
#include <chrono>
#include <string>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class ProxyOptions "proxyOptions.hpp" "umps/proxyServices/command/proxyOptions.hpp"
/// @brief Defines the options for the command proxy.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_uRemoteCommand
class ProxyOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ProxyOptions();
    /// @brief Copy constructor.
    ProxyOptions(const ProxyOptions &options);
    /// @brief Move constructor.
    ProxyOptions(ProxyOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options.
    ProxyOptions& operator=(const ProxyOptions &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    ProxyOptions& operator=(ProxyOptions &&options) noexcept;
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

    /// @name Ping Intervals
    /// @{

    /// @brief The proxy pings connected modules periodically to verify that
    ///        they are still running.  If pingIntervals is, say,
    ///        {10s, 30s, 60s}, then the proxy will ping every module every
    ///        10 seconds.  If there is no response then a request will be sent
    ///        after 30s.  And if there's no response to that ping, then a
    ///        third ping request will be sent after 1 minute.  If there is
    ///        no response, then a terminate request will be sent to the 
    ///        module.
    /// @throws std::invalid_argument if pingIntervals is empty or any value
    ///         is less than or equal to 0.
    void setPingIntervals(const std::vector<std::chrono::milliseconds> &pingIntervals);
    /// @result The ping intervals.
    [[nodiscard]] std::vector<std::chrono::milliseconds> getPingIntervals() const noexcept;
    /// @}

    /// @brief Loads proxy options from an initialization file.
    /// @param[in] iniFile  The name of the initialization file.
    /// @param[in] section  The section of the ini file from which to
    ///                     read variables.
    void parseInitializationFile(const std::string &iniFile,
                                 const std::string &section);

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~ProxyOptions();
    /// @}
private:
    class ProxyOptionsImpl;
    std::unique_ptr<ProxyOptionsImpl> pImpl;
};
}
#endif
