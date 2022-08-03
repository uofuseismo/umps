#ifndef UMPS_PROXYSERVICES_INCREMENTER_REQUESTOR_HPP
#define UMPS_PROXYSERVICES_INCREMENTER_REQUESTOR_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Request;
 }
 namespace ProxyServices::Incrementer
 {
  class RequestorOptions;
  class ItemsRequest;
  class ItemsResponse;
  class IncrementRequest;
  class IncrementResponse;
 }
}
namespace UMPS::ProxyServices::Incrementer
{
/// @class Requestor "requestor.hpp" "umps/proxyServices/incrementer/requestor.hpp"
/// @brief A requestor that will interact with the incrementer service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Requestor
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Requestor();
    /// @brief Constructor with a given logger.
    /// @param[in] logger  The logger.
    explicit Requestor(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context.
    /// @param[in] context  The ZeroMQ context to use.
    explicit Requestor(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given context and logger.
    Requestor(std::shared_ptr<UMPS::Messaging::Context> &context,
              std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] requestor  The request class from which to initialize
    ///                           this class.  On exit, request's behavior is
    ///                           undefined.
    Requestor(Requestor &&requestor) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Move assignment operator.
    /// @param[in,out] requestor  The request class whose memory will be moved
    ///                           to this.  On exit, request's behavior is
    ///                           undefined.
    Requestor& operator=(Requestor &&requestor) noexcept;
    /// @result The memory from request moved to this.
    /// @} 

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the request.
    /// @param[in] options   The request options.
    /// @throws std::invalid_argument if the endpoint.
    void initialize(const RequestorOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Request getSocketDetails() const;
    /// @}

    /// @name Step 2: Request 
    /// @{

    /// @brief Performs a blocking request for available items being
    ///        incremented.
    /// @param[in] request  The items request to make to the server via
    ///                     the router. 
    /// @result The response to the items request from the server.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<ItemsResponse> request(const ItemsRequest &request);
    /// @brief Performs a blocking request for the next value of the item.
    /// @param[in] request  The increment request to make to the server via the
    ///                     router.
    /// @result The response to the increment request from the server.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<IncrementResponse> request(const IncrementRequest &request);
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
    ~Requestor();
    /// @}

    Requestor(const Requestor &request) = delete;
    Requestor& operator=(const Requestor &request) = delete;
private:
    class RequestorImpl;
    std::unique_ptr<RequestorImpl> pImpl;
};
}
#endif
