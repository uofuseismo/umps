#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_REQUEST_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_REQUEST_HPP
#include <memory>
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Services::ConnectionInformation
 {
  class RequestOptions;
  class Details;
  namespace SocketDetails
  {
  //class 
  }
 }
}
namespace UMPS::Services::ConnectionInformation
{
/// @class RequestOptions "requestOptions.hpp" "umps/services/connectionInformation/requestOptions.hpp"
/// @brief This defines the options for the connection information client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Request
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor
    Request();
    /// @brief Move constructor.
    /// @param[in,out] request  The request class from which to initialize
    ///                         this class.  On exit, request's behavior is
    ///                         undefined.
    Request(Request &&request) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] request  The request class whose memory will be moved
    ///                         to this.  On exit, request's behavior is
    ///                         undefined.
    /// @result The memory from request moved to this.
    Request& operator=(Request &&request) noexcept;
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the requestor.
    /// @throws std::invalid_argument if the end point was not set.
    void initialize(const RequestOptions &options);
    /// @result True indicates the requestor is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Step 2: Querying the Service
    /// @{
    /// @result The connection details for all services.
    /// @throws std::runtime_error if \c isInitialized() is false.
    std::vector<Details> getDetails() const;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Disconnects from the service.
    void disconnect();
    /// @brief Destructor.
    ~Request();
    /// @}

    Request(const Request &request) = delete;
    Request& operator=(const Request &request) = delete;
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl;
};
}
#endif
