#ifndef UMPS_MESSAGING_ROUTERDEALER_REQUEST_HPP
#define UMPS_MESSAGING_ROUTERDEALER_REQUEST_HPP
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
  class SensorRequest;
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

    Request();
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

    /// @brief Performs a blocking request of from the router.
    /// @param[in] request  The request to make to the server via the router. 
    /// @result The response to the request from the server via the
    ///         router-dealer.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]]
    std::unique_ptr<UMPS::MessageFormats::IMessage> request(
        const MessageFormats::IMessage &request);
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
    Request(Request &&request) noexcept = delete;
    Request& operator=(const Request &request) = delete;
    Request& operator=(Request &&request) noexcept = delete;
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl;
};
}
#endif
