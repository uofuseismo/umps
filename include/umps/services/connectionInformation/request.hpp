#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_REQUEST_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_REQUEST_HPP
#include <memory>
#include <vector>
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
   class XPublisher;
   class XSubscriber;
   class Router;
   class Dealer;
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
    /// @brief Constructs the class with a given logger.
    explicit Request(std::shared_ptr<UMPS::Logging::ILog> &logger);
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

    /// @name Initialization
    /// @{

    /// @brief Initializes the requestor.
    /// @throws std::invalid_argument if the end point was not set.
    void initialize(const RequestOptions &options);
    /// @result True indicates the requestor is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Query All Connections
    /// @{

    /// @brief Gets the connection details for all availble connections.
    /// @result The connection details for all services.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::vector<Details> getAllConnectionDetails() const;

    /// @name Query Proxy Broadcasts
    /// @{

    /// @brief Gets the socket details to which data publishers will connect.
    /// @param[in] name  The name of the proxy broadcast.
    /// @throws std::runtime_error if the proxy broadcast name does not
    ///         exist or the \c isInitialized() is false.
    [[nodiscard]] SocketDetails::XSubscriber getProxyBroadcastFrontendDetails(const std::string &name) const;
    /// @brief Gets the socket details to which data consumers will connect.
    /// @param[in] name  The name of the proxy broadcast.
    /// @throws std::runtime_error if the proxy broadcast name does not
    ///         exist or the \c isInitialized() is false.
    [[nodiscard]] SocketDetails::XPublisher  getProxyBroadcastBackendDetails(const std::string &name) const;
    /// @}

    /// @name Query Proxy Services
    /// @{
 
    /// @brief Gets the socket details to which clients will connect.
    /// @param[in] name  The name of the proxy service.
    /// @throws std::runtime_error if the proxy service name does not exist
    ///         or the \c isInitialized() is false.
    [[nodiscard]] SocketDetails::Router getProxyServiceFrontendDetails(const std::string &name) const;
    /// @brief Gets the socket details to which servers will connect.
    /// @param[in] name  The name of the proxy service.
    /// @throws std::runtime_error if the proxy service name does not exist
    ///         or the \c isInitialized() is false.
    [[nodiscard]] SocketDetails::Dealer getProxyServiceBackendDetails(const std::string &name) const; 
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
