#ifndef UMPS_PROXYBROADCASTS_PICK_SUBSCRIBER_HPP
#define UMPS_PROXYBROADCASTS_PICK_SUBSCRIBER_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
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
 namespace ProxyBroadcasts::Pick
 {
  class SubscriberOptions;
 }
 namespace MessageFormats
 {
  class Pick;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Subscriber;
 }
}
namespace zmq 
{
 class context_t;
}
namespace UMPS::ProxyBroadcasts::Pick
{
/// @class Subscriber "subscriber.hpp" "umps/proxyBroadcasts/pick/subscriber.hpp"
/// @brief A convenience function to subscribe to a pick broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Subscriber
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Subscriber();
    /// @brief Constructs a subscriber with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a subscriber with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Subscriber(std::shared_ptr<zmq::context_t> &context);
    /// @brief Construtcs a subscriber with a given ZeroMQ context and logger.
    Subscriber(std::shared_ptr<zmq::context_t> &context,
               std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] subscriber  The subscriber from which to initialize this
    ///                            class.  On exit, subscriber's behavior is
    ///                            undefined. 
    Subscriber(Subscriber &&subscriber) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment.
    /// @param[in,out] subscriber  The subscriber whose memory will be moved to
    ///                            this.  On exit, subscriber's behavior is
    ///                            undefined.
    Subscriber& operator=(Subscriber &&subscriber) noexcept;
    /// @}

    /// @brief Sets the subscriber options.
    /// @throws std::invalid_argument if options.haveAddress() is false.
    /// @throws std::runtime_error if the connection cannot be established.
    void initialize(const SubscriberOptions &options);
    /// @result True indicates that the subscriber is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The connection details for this socket.
    /// @throws std::runtime_error if \c isInitialized() is false. 
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Subscriber getSocketDetails() const;

    /// @brief Receives a pick message.
    /// @throws std::invalid_argument if the message cannot be serialized.
    /// @throws std::runtime_error if \c isIinitialized() is false.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::Pick> receive() const;

    /// @brief Destructor. 
    ~Subscriber();

    Subscriber(const Subscriber &) = delete;
    Subscriber& operator=(const Subscriber &) = delete;
private:
    class SubscriberImpl;
    std::unique_ptr<SubscriberImpl> pImpl;
};
}
#endif
