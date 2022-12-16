#ifndef UMPS_PROXY_SERVICES_COMMAND_REQUESTOR_OPTIONS_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REQUESTOR_OPTIONS_HPP
#include <memory>
#include <chrono>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
 namespace Messaging::RequestRouter
 {
  class RequestOptions;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class RequestorOptions "requestorOptions.hpp" "umps/proxyServices/command/requestorOptions.hpp"
/// @brief This sets the parameters for the utility that will allow users to 
///        interact with remote modules.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup UMPS_ProxyServices_Command
class RequestorOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RequestorOptions();
    /// @brief Copy constructor.
    RequestorOptions(const RequestorOptions &options);
    /// @brief Move constructor.
    RequestorOptions(RequestorOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options. 
    RequestorOptions& operator=(const RequestorOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    RequestorOptions& operator=(RequestorOptions &&options) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief Sets the address of the proxy to which commands will be submitted.
    /// @param[in] address   The address.
    /// @throws std::invalid_argument if name is empty.
    void setAddress(const std::string &address);
    /// @result True indicates the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name Optional Parameters
    /// @{

    /// @brief Sets the ZeroMQ authentication options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options) noexcept;
    /// @brief Sets the time out for receiving replies from the local module.
    /// @param[in] timeOut  The receive time out.  If this is negative then
    ///                     this program will hang indefinitely until the
    ///                     program you are interacting with responds - this is
    ///                     problematic if your thread has other things to do.
    ///                     If this zero then this requestor will return
    ///                     immediately and probably miss the response.
    ///                     In general, it's a good idea to wait a few
    ///                     hundredths of second for a response.
    void setReceiveTimeOut(const std::chrono::milliseconds &timeOut);
    /// @}

    /// @result The requestor options.
    /// @throws std::runtime_error if \c haveAddress() is false.
    [[nodiscard]] UMPS::Messaging::RequestRouter::RequestOptions getOptions() const;

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~RequestorOptions(); 
    /// @} 
private:
    class RequestorOptionsImpl;
    std::unique_ptr<RequestorOptionsImpl> pImpl;
};
}
#endif
