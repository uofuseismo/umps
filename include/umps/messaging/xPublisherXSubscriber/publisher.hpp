#ifndef UMPS_MESSAGING_XPUBLISHERXSUBSCRIBER_PUBLISHER_HPP
#define UMPS_MESSAGING_XPUBLISHERXSUBSCRIBER_PUBLISHER_HPP
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
 namespace Messaging
 {
  namespace Authentication
  {
   class ZAPOptions;
   namespace Certificate
   {
    class Keys;
    class UserNameAndPassword;
   }
  }
  namespace XPublisherXSubscriber
  {
   class PublisherOptions;
  }
 }
}
namespace zmq
{
 class context_t;
}
namespace UMPS::Messaging::XPublisherXSubscriber
{
/// @class Publisher "publisher.hpp" "umps/messaging/publisherSubscriber/publisher.hpp"
/// @brief A ZeroMQ publisher.
/// @detail This is the publisher in a XPUB/XSUB socket.  Here, we assume the
///         proxy socket is the stable point of our network so with this
///         publisher you connect to a socket whereas with the publisher in 
///         the traditional pub/sub you bind to a socke and allow subscribers
///         to connect to you.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Publisher
{
public:
    /// @brief Default constructor.
    Publisher();
    /// @brief Constructs a publisher with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger);
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

    /// @name Initialization
    /// @{
    /// @brief Initializes the publisher.
    /// @param[in] options   The options must contain the socket's address.
    ///                      Additionally, options will contain the security
    ///                      protocol.
    /// @throws std::invalid_argument the socket addres is not specified.
    /// @throws std::runtime_error if the creation of the publisher fails.
    void initialize(const PublisherOptions &options);
    /// @result The security level of the connection.
    [[nodiscard]] Authentication::SecurityLevel getSecurityLevel() const noexcept;

    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The address to which the consumers will subscribe.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    [[nodiscard]] std::string getFrontendAddress() const;
    /// @}

    /// @brief Sends a message.
    /// @param[in] message  The message to send.
    /// @throws std::invalid_argument if the message cannot be serialized.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This will serialize the message prior to sending it.
    void send(const MessageFormats::IMessage &message);

    /// @brief Closes the connection.
    /// @note The class will need to be initialized again to restore the
    ///       connection.
    void disconnect(); 

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