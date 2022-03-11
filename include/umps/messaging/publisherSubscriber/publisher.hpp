#ifndef UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#define UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
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
  namespace PublisherSubscriber
  {
   class PublisherOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Publisher;
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
/// @details You can think of a publisher as a radio broadcast.  If you, as 
///          a subscriber, are tuned in then you'll hear the broadcast.  
///          However, if you tune in late or turn off your radio then
///          you'll miss those sections of the broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_PubSub_PubSub
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
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Publisher(std::shared_ptr<zmq::context_t> &context);
    /// @brief Construtcs a publisher with a given ZeroMQ context and logger.
    Publisher(std::shared_ptr<zmq::context_t> &context,
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

    /// @brief Initializes the publisher.
    /// @param[in] options  The publisher options.  This must have an address.
    /// @throws std::invalid_argument if \c options.haveAddress().
    void initialize(const PublisherOptions &options);
    /// @result True indicates that the subscriber is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The security level of the connection.
    [[deprecated]] [[nodiscard]] Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result The socket endpoint.
    /// @throws std::runtime_error if \c isInitialized() is true.
    [[deprecated]] [[nodiscard]] std::string getEndPoint() const;
    /// @result Gets the details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Publisher getSocketDetails() const;

    /// @brief Sends a message.  This will serialize the message.
    /// @param[in] message  The message to send.
    /// @throws std::runtime_error if the class is not initialized.
    /// @throws std::invalid_argument if the message cannot be serialized.
    void send(const MessageFormats::IMessage &message);

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
