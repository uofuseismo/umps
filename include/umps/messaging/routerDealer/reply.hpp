#ifndef UMPS_MESSAGING_REQUESTROUTER_REPLY_HPP
#define UMPS_MESSAGING_REQUESTROUTER_REPLY_HPP
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
   class ReplyOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Reply;
 }
}
namespace UMPS::Messaging::RouterDealer
{
/// @class Reply "reply.hpp" "umps/messaging/routerDealer/reply.hpp"
/// @brief A ZeroMQ reply for use in the router-dealer.
/// @details The router-dealer pattern is useful for when we want an 
///          asynchronous communication between servers and clients.
///          This class represents the server.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RouterDealer
class Reply
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Reply();
    /// @brief Constructs a request socket with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Reply(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a reply socket with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<UMPS::Messaging::Context> (0).
    explicit Reply(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructs a reply socket with the given context and logger.
    Reply(std::shared_ptr<UMPS::Messaging::Context> &context,
          std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the reply.
    /// @param[in] options   The reply options.
    /// @throws std::invalid_argument if the endpoint or callback is not set.
    void initialize(const ReplyOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The security level of the connection.
    [[deprecated]] [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result The connection string.
    /// @throws std::runtime_error if \c isConnected() is false.
    [[deprecated]] [[nodiscard]] std::string getConnectionString() const;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Reply getSocketDetails() const;
    /// @}

    /// @name Step 2: Start the Reply Service
    /// @{

    /// @brief Starts the reply service.  The service will poll on messages
    ///        from the dealer, process the messages with the provided callback,
    ///        and return the result to the dealer to propagate via the router
    ///        back to the client. 
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start();

    /// @result True indicates that the reply service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @}

    /// @name Step 3: Stop the Reply Service
    /// @{

    /// @brief This will stop the reply service.
    void stop();
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Reply();
    /// @}

    Reply(const Reply &reply) = delete;
    Reply(Reply &&reply) noexcept = delete;
    Reply& operator=(const Reply &reply) = delete;
    Reply& operator=(Reply &&reply) noexcept = delete;
private:
    class ReplyImpl;
    std::unique_ptr<ReplyImpl> pImpl;
};
}
#endif
