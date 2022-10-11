#ifndef UMPS_SERVICES_COMMAND_LOCAL_REQUESTOR_OPTIONS_HPP
#define UMPS_SERVICES_COMMAND_LOCAL_REQUESTOR_OPTIONS_HPP
#include <memory>
#include <chrono>
namespace UMPS::ProxyServices::Command
{
 class RequestorOptions;
}
namespace UMPS::Services::Command
{
/// @class LocalRequestorOptions "localRequestorOptions.hpp" "umps/services/command/localRequestorOptions.hpp"
/// @brief This sets the parameters for the utility that will allow users to 
///        interact locally with a program.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class LocalRequestorOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    LocalRequestorOptions();
    /// @brief Copy constructor.
    LocalRequestorOptions(const LocalRequestorOptions &options);
    /// @brief Move constructor.
    LocalRequestorOptions(LocalRequestorOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options. 
    LocalRequestorOptions& operator=(const LocalRequestorOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    LocalRequestorOptions& operator=(LocalRequestorOptions &&options) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief Sets the locally running module name with which to interact.
    /// @param[in] name  The name of the module.
    /// @throws std::invalid_argument if name is empty.
    void setModuleName(const std::string &name);
    /// @result The name of the local module with which to interact.
    [[nodiscard]] std::string getModuleName() const;
    /// @result True indicates the module name was set.
    [[nodiscard]] bool haveModuleName() const noexcept;
    /// @result The name of the IPC file.
    /// @Throws std::runtime_erorr if \c haveModuleName() is false.
    [[nodiscard]] std::string getIPCFileName() const;
    /// @}

    /// @name Optional Parameters
    /// @{

    /// @brief Sets the IPC directory that will house IPC files.
    /// @param[in] directory  The directory where IPC communication file
    ///                       will exist.
    /// @throws std::invalid_argument if the directory does not exist.
    void setIPCDirectory(const std::string &directory);
    /// @result The directory where the IPC communication files will exist.
    [[nodiscard]] std::string getIPCDirectory() const noexcept;
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

    /// @result The request options.
    [[nodiscard]] ProxyServices::Command::RequestorOptions getOptions() const;

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~LocalRequestorOptions(); 
    /// @} 
private:
    class LocalRequestorOptionsImpl;
    std::unique_ptr<LocalRequestorOptionsImpl> pImpl;
};
}
#endif
