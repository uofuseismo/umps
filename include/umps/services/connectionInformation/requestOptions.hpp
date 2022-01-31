#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_REQUESTOPTIONS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_REQUESTOPTIONS_HPP
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
/// @class RequestOptions "requestOptions.hpp" "umps/services/connectionInformation/requestOptions.hpp"
/// @brief This defines the options for the connection information client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RequestOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor
    RequestOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The request options from which to initialize
    ///                     this class.
    RequestOptions(const RequestOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The request options from which to initialize
    ///                         this class.  On exit, options's behavior is
    ///                         undefined.
    RequestOptions(RequestOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the options.
    RequestOptions& operator=(const RequestOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved 
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from moved from options to this.
    RequestOptions& operator=(RequestOptions &&options) noexcept;
    /// @}

    /// @brief Sets the client access address of the uOperator's connection
    ///        information service.
    /// @param[in] address  The client access address.
    /// @throws std::invalid_argument if the address is empty.
    void setEndPoint(const std::string &address);
    /// @brief Sets the ZeroMQ authentication protocol options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    /// @brief Sets the request time out.  This is useful when the
    ///        requesting thread has other responsibilities.
    /// @param[in] timeOut  The time out.  If this is negative the request
    ///                     will block indefinitely until a response is
    ///                     received.  If this is 0 then the requestor
    ///                     will return immediately.  Otherwise, the requestor 
    ///                     will wait this many milliseconds prior to returning.
    void setTimeOut(const std::chrono::milliseconds timeOut) noexcept;
    /// @result The request options for initializing the requestor.
    [[nodiscard]] UMPS::Messaging::RequestRouter::RequestOptions getRequestOptions() const noexcept;
    /// @brief Initializes the class from an ini file.
    /// @param[in] iniFile  The name of the initialization file to parse.
    /// @throws std::runtime_error if the uOperator.ipAddress cannot be found. 
    void parseInitializationFile(const std::string &iniFile);

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RequestOptions();
    /// @}
private:
    class RequestOptionsImpl;
    std::unique_ptr<RequestOptionsImpl> pImpl;
};
}
#endif
