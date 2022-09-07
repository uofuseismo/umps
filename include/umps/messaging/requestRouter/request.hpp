#ifndef UMPS_MESSAGING_REQUESTROUTER_REQUEST_HPP
#define UMPS_MESSAGING_REQUESTROUTER_REQUEST_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Messaging
 {
  class Context;
  namespace RequestRouter
  {
   class RequestOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Request;
 }
}
namespace UMPS::Messaging::RequestRouter
{
/// @class Request "request.hpp" "umps/messaging/requestRouter/request.hpp"
/// @brief A ZeroMQ request for use in the request-router combination.
/// @details The request-router pattern is useful for when we want an 
///          asynchronous server that can talk to multiple request clients
///          at the same time.  This class represents the client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RequestRouter
class Request
{
public:
    /// @name Constructors
    /// @{

    Request();
    /// @brief Constructs a request socket with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Request(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a request socket with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<UMPS::Messaging::Context> (0).
    explicit Request(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructs a request socket with the given context and logger.
    Request(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger);

    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the request.
    /// @param[in] options   The request options.
    /// @throws std::invalid_argument if the endpoint.
    void initialize(const RequestOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept; 
    /// @result The security level of the connection.
    [[deprecated]] [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result the connection details.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Request getSocketDetails() const;

    /// @}

    /// @name Step 2: Request
    /// @{

    /// @brief Performs a blocking request of from the router.
    /// @param[in] request  The request to make to the server via the router. 
    /// @result The response to the request from the server (via the router).
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]]
    std::unique_ptr<UMPS::MessageFormats::IMessage> request(
        const MessageFormats::IMessage &request);

    /// @}

    /// @name Step 3: Disconnect
    /// @{

    /// @brief Disconnects.
    /// @note This will automatically be done by the destructor.
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
