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
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Messaging
 {
  class Context;
  namespace RouterDealer
  {
   class RequestOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Request;
 }
}
namespace UMPS::Messaging::RouterDealer
{
/// @class Request "request.hpp" "umps/messaging/requestRouter/request.hpp"
/// @brief A ZeroMQ request for use in the router-dealer combination.
/// @details The router-dealer pattern is useful for when we want multiple
///          servers processing the requests of multiple clients asynchronously.
///          This class represents the client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RouterDealer
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
    /// @brief Move constructor.
    /// @param[in,out] request  The requestor class from which to initialize
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

    /// @brief Initializes the request.
    /// @param[in] options   The request options.
    /// @throws std::invalid_argument if the endpoint.
    void initialize(const RequestOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept; 
    /// @brief Sets the message format for receiving responses.
    //void setResponse(std::unique_ptr<UMPS::MessageFormats::IMessage> &message);
    /// @result The security level of the connection.
    [[deprecated]] [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result The connection string.
    /// @throws std::runtime_error if \c isConnected() is false.
    [[deprecated]] [[nodiscard]] std::string getConnectionString() const;
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
    Request& operator=(const Request &request) = delete;
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl;
};
}
#endif
