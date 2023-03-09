#ifndef UMPS_MESSAGING_XPUBLISHER_XSUBSCRIBER_PUBLISHER_HPP
#define UMPS_MESSAGING_XPUBLISHER_XSUBSCRIBER_PUBLISHER_HPP
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
  namespace XPublisherXSubscriber
  {
   class PublisherOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class XPublisher;
 }
}
namespace UMPS::Messaging::XPublisherXSubscriber
{
/// @class Publisher "publisher.hpp" "umps/messaging/publisherSubscriber/publisher.hpp"
/// @brief A ZeroMQ publisher.
/// @details This is the publisher in a XPUB/XSUB socket.  Here, we assume the
///          proxy socket is the stable point of our network so with this
///          publisher you connect to a socket whereas with the publisher in 
///          the traditional pub/sub you bind to a socke and allow subscribers
///          to connect to you.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_PubSub_XPubXSub
class Publisher
{
public:
    /// @name Constructors
    /// @{

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
    ///       auto context = std::shared_ptr<UMPS::Messaging::Context> (0).
    explicit Publisher(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructs a publisher with a given ZeroMQ context and logger.
    Publisher(std::shared_ptr<UMPS::Messaging::Context> &context,
              std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] publisher  The publisher class from which to initialize
    ///                           this class.  On exit, publisher's behavior is
    ///                           undefined.
    Publisher(Publisher &&publisher) noexcept;
     /// @}

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
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::XPublisher getSocketDetails() const;
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
    //Publisher(Publisher &&publisher) noexcept = delete;
    Publisher& operator=(const Publisher &publisher) = delete;
    //Publisher& operator=(Publisher &&publisher) noexcept = delete;
private:
    class PublisherImpl;
    std::unique_ptr<PublisherImpl> pImpl;
};
}
#endif
