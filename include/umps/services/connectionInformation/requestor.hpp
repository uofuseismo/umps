#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_REQUESTOR_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_REQUESTOR_HPP
#include <memory>
#include <vector>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
 namespace Services::ConnectionInformation
 {
  class RequestorOptions;
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
/// @class Requestor "requestor.hpp" "umps/services/connectionInformation/requestor.hpp"
/// @brief This defines the requestor that interacts with the connection
///        information service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Requestor
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor
    Requestor();
    /// @brief Constructs the class with a given logger.
    explicit Requestor(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context.
    /// @param[in] context  The ZeroMQ context to use.
    explicit Requestor(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given context and logger.
    Requestor(std::shared_ptr<UMPS::Messaging::Context> &context,
              std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] request  The request class from which to initialize
    ///                         this class.  On exit, request's behavior is
    ///                         undefined.
    Requestor(Requestor &&request) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Move assignment operator.
    /// @param[in,out] request  The request class whose memory will be moved
    ///                         to this.  On exit, request's behavior is
    ///                         undefined.
    /// @result The memory from request moved to this.
    Requestor& operator=(Requestor &&request) noexcept;
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes the requestor.
    /// @throws std::invalid_argument if the end point was not set.
    void initialize(const RequestorOptions &options);
    /// @result True indicates the requestor is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The requestor options.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] RequestorOptions getRequestorOptions() const;
    /// @result The requestor's ZAP options.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const;
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
    ~Requestor();
    /// @}

    Requestor(const Requestor &) = delete;
    Requestor& operator=(const Requestor &) = delete;
private:
    class RequestorImpl;
    std::unique_ptr<RequestorImpl> pImpl;
};
/// @brief Creates a connection information requestor from an
///        initialization file.
/// @param[in] requestor  The connection information requestor.
/// @param[in] iniFile    The initialization file.
/// @param[in] section    The section of the initialization file with the
///                       heartbeat options.
/// @param[in] context    The ZeroMQ context.
/// @param[in] logger     The application's logger.
/// @result On successful exit, this is a connection information requestor that
///         is connected and ready to query UMPS for broadcasts, services, etc.
std::unique_ptr<Requestor>
    createRequestor(const std::string &iniFile,
                    const std::string &section = "uOperator",
                    std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                    std::shared_ptr<UMPS::Logging::ILog> logger = nullptr);

}
#endif
