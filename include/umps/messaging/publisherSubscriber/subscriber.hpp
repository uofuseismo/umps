#ifndef UMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#define UMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
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
 namespace Messaging::PublisherSubscriber
 {
  class SubscriberOptions;
 }
}
namespace zmq
{
 class context_t;
}
namespace UMPS::Messaging::PublisherSubscriber
{
/// @class Subscriber "subscriber.hpp" "umps/messaging/publisherSubscriber/subscriber.hpp"
/// @brief The subscriber in a publisher/subscriber messaging pattern.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Subscriber
{
public:
    /// @name Constructors
    /// @{
    /// @brief Default constructor.
    Subscriber();
    /// @brief Constructor with specified logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a specified context.
    /// @param[i] context  The ZeroMQ context.
    explicit Subscriber(std::shared_ptr<zmq::context_t> &context);
    /// @brief Construtcs a publisher with a given ZeroMQ context and logger.
    Subscriber(std::shared_ptr<zmq::context_t> &context,
               std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] subscriber  The subscriber class from which to initialize
    ///                            this class.  On exit, subscriber's behavior
    ///                            is undefined.
    Subscriber(Subscriber &&subscriber) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] subscriber  The subscriber class whose memory will be
    ///                            moved to this.  On exit, the subscriber's
    ///                            behavior is undefined.
    /// @result The memory from subscriber moved to this.
    Subscriber& operator=(Subscriber &&subscriber) noexcept;
    /// @}

    /// @brief Initializes the subscriber.
    /// @param[in] options  The subscriber options.  This must have an address
    ///                     and message types.
    /// @throws std::invalid_argument if \c options.haveAddress() or 
    ///         options.haveMessageTypes() is false.
    /// @throws std::runtime_error if the connection cannot be established.
    void initialize(const SubscriberOptions &options);
    /// @result True indicates that the subscriber is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The security level of the connection.
    [[nodiscard]] Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result The socket endpoint.
    /// @throws std::runtime_error if \c isInitialized() is true.
    [[nodiscard]] std::string getEndPoint() const;

    /// @brief Receives a message.
    /// @throws std::invalid_argument if the message cannot be serialized.
    [[nodiscard]] std::unique_ptr<MessageFormats::IMessage> receive() const;

    /// @brief Disconnects the subscriber.
    /// @note The class will have to be reinitialized to connect.
    void disconnect();

    /// @name Destructors
    /// @{
    /// @brief Destructor.
    ~Subscriber();
    /// @}
    /// Delete some functions
    Subscriber(const Subscriber &subscriber) = delete;
    Subscriber& operator=(const Subscriber &subscriber) = delete;
private:
    class SubscriberImpl;
    std::unique_ptr<SubscriberImpl> pImpl;
};
}
#endif
