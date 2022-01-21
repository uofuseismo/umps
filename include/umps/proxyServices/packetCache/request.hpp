#ifndef UMPS_PROXYSERVICES_PACKETCACHE_REQUEST_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_REQUEST_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Request;
 }
 namespace ProxyServices::PacketCache
 {
  class RequestOptions;
  class DataRequest;
  template<class T> class DataResponse;
  class SensorRequest;
  class SensorResponse;
 }
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class Request "request.hpp" "umps/proxyServices/packetCache/request.hpp"
/// @brief A ZeroMQ requestor from which to query the packet cache.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Request
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Request();
    /// @brief Constructor with a given logger.
    /// @param[in] logger  The logger.
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
    Request& operator=(Request &&request) noexcept;
    /// @result The memory from request moved to this.
    /// @} 

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the request.
    /// @param[in] options   The request options.
    /// @throws std::invalid_argument if the endpoint.
    void initialize(const RequestOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Request getSocketDetails() const;
    /// @}

    /// @name Step 2: Request 
    /// @{

    /// @brief Performs a blocking request for available sensors
    ///        in the packet cache.
    /// @param[in] request  The sensor request to make to the server via
    ///                     the router. 
    /// @result The response to the sensor request from the server.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<SensorResponse> request(const SensorRequest &request);
    /// @brief Performs a blocking request for data in the packet cache. 
    /// @param[in] request  The data request to make to the server via the
    ///                     router.
    /// @result The response to the data request from the server.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<DataResponse<double>> request(const DataRequest &request);
    /// @}

    /// @name Step 3: Disconnecting
    /// @{

    /// @brief Disconnects the requestor from the router-dealer.
    /// @note This step is optional as it will be done by the destructor.
    void disconnect();
    /// @}
 
    /// @name Destructors
    /// @{

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
