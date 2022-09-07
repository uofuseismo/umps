#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_REQUESTOROPTIONS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_REQUESTOROPTIONS_HPP
#include <memory>
#include <chrono>
namespace UMPS
{
 namespace Messaging::RequestRouter
 {
  class RequestOptions;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::Services::ConnectionInformation
{
/// @class RequestorOptions "requestorOptions.hpp" "umps/services/connectionInformation/requestorOptions.hpp"
/// @brief This defines the options for the connection information client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RequestorOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor
    RequestorOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The request options from which to initialize
    ///                     this class.
    RequestorOptions(const RequestorOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The request options from which to initialize
    ///                         this class.  On exit, options's behavior is
    ///                         undefined.
    RequestorOptions(RequestorOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the options.
    RequestorOptions& operator=(const RequestorOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved 
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from moved from options to this.
    RequestorOptions& operator=(RequestorOptions &&options) noexcept;
    /// @}

    /// @brief Sets the client access address of the uOperator's connection
    ///        information service.
    /// @param[in] address  The client access address.
    /// @throws std::invalid_argument if the address is empty.
    void setAddress(const std::string &address);
    /// @brief Sets the ZeroMQ authentication protocol options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    /// @brief Sets the request time out.  This is useful when the
    ///        requesting thread has other responsibilities.
    /// @param[in] timeOut  The time out.  If this is negative the request
    ///                     will block indefinitely until a response is
    ///                     received.  If this is 0 then the requestor
    ///                     will return immediately.  Otherwise, the requestor 
    ///                     will wait this many milliseconds prior to returning.
    void setTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The request options for initializing the requestor.
    [[nodiscard]] UMPS::Messaging::RequestRouter::RequestOptions getRequestOptions() const noexcept;
    /// @brief Initializes the class from an ini file.
    /// @param[in] iniFile  The name of the initialization file to parse.
    /// @param[in] section  The section of the initialization file with the
    ///                     uOperator connection details.
    /// @throws std::runtime_error if the section.address cannot be found. 
    void parseInitializationFile(const std::string &iniFile,
                                 const std::string &section = "uOperator");

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
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
