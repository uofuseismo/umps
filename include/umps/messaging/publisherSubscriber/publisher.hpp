#ifndef UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#define UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#include <memory>
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
namespace UMPS::Messaging::PublisherSubscriber
{
/// @class Publisher "publisher.hpp" "umps/messaging/publisherSubscriber/publisher.hpp"
/// @brief A ZeroMQ publisher.
/// @detail You can think of a publisher as a radio broadcast.  If you, as 
///         a subscriber, are tuned in then you'll hear the broadcast.  
///         However, if you tune in late or turn off your radio then
///         you'll miss those sections of the broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Publisher
{
public:
    /// @brief Default constructor.
    Publisher();
    /// @brief Constructs a publisher with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a publisher with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Publisher(std::shared_ptr<zmq::context_t> &context);
    /// @brief Move constructor.
    /// @param[in,out] publisher  The publisher class from which to initialize
    ///                           this class.  On exit, publisher's behavior is
    ///                           undefined.
    Publisher(Publisher &&publisher) noexcept;

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] publisher  The publisher class whose memory will be
    ///                           moved to this.  On exit, publisher's
    ///                           behavior is undefined.
    /// @result The memory from publisher moved to this.
    Publisher& operator=(Publisher &&publisher) noexcept;
    /// @}

    /// @brief Binds the endpoint.  This is the grasslands or strawhouse
    ///        pattern.
    /// @param[in] endPoint    The endpoint for accepting connections.
    ///                        For example, "tcp://127.0.0.1:5555".
    /// @note For more see: http://api.zeromq.org/2-1:zmq-bind. 
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint);
    /// @brief Binds the endpoint as an IP validating server or client.  This is
    ///        the strawhouse pattern that will authenticate IP addresses.
    /// @param[in] endPoint   The endpoint for accepting connections.
    /// @param[in] isAuthenticationServer  True indicates this connection is a
    ///                                    ZAP server which means this machine
    ///                                    will validate users.
    /// @param[in] zapDomain  The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if plainText.haveUserName() or
    ///         plainText.havePassword() are false.
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint,
              bool isAuthenticationServer,
              const std::string &zapDomain = "global");
    /// @brief Binds the endpoint as a plain text server or client.  This is
    ///        the stonehouse pattern.
    /// @param[in] endPoint   The endpoint for accepting connections.
    /// @param[in] plainText  The plain text credentials.
    /// @param[in] isAuthenticationServer  True indicates this connection is a
    ///                                    ZAP server which means this machine
    ///                                    will validate users.
    /// @param[in] zapDomain  The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if plainText.haveUserName() or
    ///         plainText.havePassword() are false.
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint,
              const UMPS::Messaging::Authentication::Certificate::UserNameAndPassword &plainText,
              bool isAuthenticationServer,
              const std::string &zapDomain = "global");
    /// @brief Binds the endpoint as a CURVE server.  This is the stonehouse
    ///        pattern.
    /// @param[in] endPoint   The endpoint for accepting connections.
    /// @param[in] serverKeys The public and private keys for the server.
    /// @param[in] zapDomain  The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if serverKeys.havePublicKey() or
    ///         serverKeys.havePrivateKey() is false.
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint,
              const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
              const std::string &zapDomain = "global");
    /// @brief Binds the endpoint as a CURVE client.  This is the stonehouse
    ///        pattern.
    /// @param[in] endPoint   The endpoint for accepting connections.
    /// @param[in] serverKeys The public and private keys for the server.
    /// @param[in] clientKeys The public and private keys for the client.
    /// @param[in] zapDomain  The ZeroMQ Authentication Protocol domain.
    /// @throws std::invalid_argument if serverKeys.havePublicKey() is false
    ///         or \c clientKeys.havePublicKey() or
    ///         \c clientKeys.havePrivateKey() is false.
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint,
              const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
              const UMPS::Messaging::Authentication::Certificate::Keys &clientKeys, 
              const std::string &zapDomain = "global");
    /// @result The security level of the bound socket.
    [[nodiscard]] Authentication::SecurityLevel getSecurityLevel() const noexcept;

    /// @brief Sends a message.  This will serialize the message.
    /// @param[in] message  The message to send.
    /// @throws std::invalid_argument if the message cannot be serialized.
    void send(const MessageFormats::IMessage &message);
    /// @brief This is a lower level function for sending a message.
    //void send(size_t nBytes, const char *message);
    //void send(const std::string &message);

    /// @name Destructors
    /// @{
    /// @brief Destructor.
    ~Publisher();
    /// @}

    /// Delete some functions
    Publisher(const Publisher &publisher) = delete;
    Publisher& operator=(const Publisher &publisher) = delete;
private:
    class PublisherImpl;
    std::unique_ptr<PublisherImpl> pImpl;
};
}
#endif
