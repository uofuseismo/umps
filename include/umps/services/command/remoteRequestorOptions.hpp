#ifndef UMPS_SERVICES_COMMAND_REMOTEREQUESTOROPTIONS_HPP
#define UMPS_SERVICES_COMMAND_REMOTEREQUESTOROPTIONS_HPP
#include <memory>
#include <chrono>
namespace UMPS::Messaging::RequestRouter
{
 class RequestOptions;
}
namespace UMPS::Services::Command
{
/// @class RemoteRequestorOptions "localRequestorOptions.hpp" "umps/services/command/localRequestorOptions.hpp"
/// @brief This sets the parameters for the utility that will allow users to 
///        interact locally with a program.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RemoteRequestorOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RemoteRequestorOptions();
    /// @brief Copy constructor.
    RemoteRequestorOptions(const RemoteRequestorOptions &options);
    /// @brief Move constructor.
    RemoteRequestorOptions(RemoteRequestorOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options. 
    RemoteRequestorOptions& operator=(const RemoteRequestorOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    RemoteRequestorOptions& operator=(RemoteRequestorOptions &&options) noexcept;
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
    ~RemoteRequestorOptions(); 
    /// @} 
private:
    class RemoteRequestorOptionsImpl;
    std::unique_ptr<RemoteRequestorOptionsImpl> pImpl;
};
}
#endif
