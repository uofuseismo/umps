#ifndef UMPS_MESSAGING_REQUESTROUTER_REQUEST_HPP
#define UMPS_MESSAGING_REQUESTROUTER_REQUEST_HPP
#include <memory>
#include <functional>
#include "umps/messaging/authentication/enums.hpp"
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
 namespace Messaging::Authentication::Certificate
 {
  class Keys;
  class UserNameAndPassword;
 }
}
namespace zmq
{
 class context_t;
}
namespace UMPS::Messaging::RequestRouter
{
/// @class Request "request.hpp" "umps/messaging/requestRouter/request.hpp"
/// @brief A ZeroMQ request for use in the request-router combination.
/// @detail The request-router pattern is useful for when we want an 
///         asynchronous server that can talk to multiple request clients
///         at the same time.  This class represents the client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Request
{
public:
    /// @name Constructors
    /// @{
    Request();
    /// @brief Constructs a request with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Request(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a request with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Request(std::shared_ptr<zmq::context_t> &context);
    /// @}

    /// @brief Sets the message format for receiving responses.
    void setResponse(std::unique_ptr<UMPS::MessageFormats::IMessage> &message);
    //void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
    //                                     (const std::string &, const uint8_t *, size_t)> &callback);

    /// @name Connecting
    /// @{
    /// @brief Connects to the given address.  This is the grasslands
    ///        security model.
    /// @param[in] endpoint  The endpoint to which to connect.
    /// @throws std::runtime_error if the connection fails.
    void connect(const std::string &endpoint);
    /// @brief Connects to the given address.  This is a strawhouse pattern
    ///        that can validate IP addresses.
    /// @param[in] endpoint  The endpoint to which to connect.
    /// @param[in] isAuthenticationServer  True indicates this connection is a
    ///                                    ZAP server which means this machine
    ///                                    will validate users.
    /// @param[in] zapDomain  The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if the endpoint or the zapDomain is empty
    ///         and this is an authenticaiton server.
    /// @throws std::runtime_error if the connection fails.
    void connect(const std::string &endpoint,
                 bool isAuthenticationServer,
                 const std::string &zapDomain = "global");
    /// @brief Connects to the given address.  This is a woodhouse pattern
    ///        that can validate IP addresses and usernames and passwords.
    /// @param[in] endpoint  The endpoint to which to connect.
    /// @param[in] isAuthenticationServer  True indicates this connection is a
    ///                                    ZAP server which means this machine
    ///                                    will validate users.
    /// @param[in] zapDomain  The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if the username and password are not set
    ///         and this is an authentciation server.
    /// @throws std::invalid_argument if the endpoint or the zapDomain is empty
    ///         and this is an authenticaiton server.
    /// @throws std::runtime_error if the connection fails.
    void connect(const std::string &endpoint,
                 const Authentication::Certificate::UserNameAndPassword &credentials,
                 bool isAuthenticationServer,
                 const std::string &zapDomain = "global"); 
    /// @brief Connects to the given address as a CURVE server.  This is a
    ///        stonehouse pattern that can validate IP addresses and public
    ///        keys.
    /// @param[in] endpoint    The endpoint to which to connect.
    /// @param[in] serverKeys  The server's public and private key. 
    /// @param[in] zapDomain   The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if the server's private key is not set.
    void connect(const std::string &endpoint,
                 const Authentication::Certificate::Keys &serverKeys,
                 const std::string &zapDomain = "global");
    /// @brief Connects to the given address as a CURVE client.  This is a
    ///        stonehouse pattern that can validate the client's IP address and
    ///        credentials.
    /// @param[in] endpoint    The endpoint to which to connect.
    /// @param[in] serverKeys  The server's public key.
    /// @param[in] serverKeys  The clients's public and private key.
    /// @param[in] zapDomain   The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if the server's public key is not set,
    ///         the client's public and private key are not set, the endpoint
    ///         is empty, or the zapDomain is empty. 
    /// @throws std::runtime_error if the connection cannot be established.
    void connect(const std::string &endpoint,
                 const Authentication::Certificate::Keys &serverKeys,
                 const Authentication::Certificate::Keys &clientKeys,
                 const std::string &zapDomain = "global");

    /// @result The security level of the connection.
    [[nodiscard]] Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result True indicates that the requester is connected.
    [[nodiscard]] bool isConnected() const noexcept;
    /// @result The connection string.
    /// @throws std::runtime_error if \c isConnected() is false.
    [[nodiscard]] std::string getConnectionString() const;
    /// @}

    /// @brief Performs a blocking request of from the router.
    /// @param[in] request  The request to make to the server via the router. 
    /// @result The response to the request from the server (via the router).
    [[nodiscard]]
    std::unique_ptr<UMPS::MessageFormats::IMessage> request(
        const MessageFormats::IMessage &request);

    /// @brief Disconnects.
    void disconnect();

    /// @name Destructors
    /// @{
    ~Request();
    /// @}
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl;
};
}
#endif
