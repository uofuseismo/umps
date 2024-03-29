#ifndef UMPS_PROXY_BROADCASTS_HEARTBEAT_SUBSCRIBER_HPP
#define UMPS_PROXY_BROADCASTS_HEARTBEAT_SUBSCRIBER_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace ProxyBroadcasts::Heartbeat
 {
  class Status; 
  class SubscriberOptions;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Subscriber;
 }
}
namespace UMPS::ProxyBroadcasts::Heartbeat
{
/// @class Subscriber "subscriber.hpp" "umps/proxyBroadcasts/heartbeat/subscriber.hpp"
/// @brief A convenience function to subscribe to a heartbeat broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup UMPS_ProxyBroadcasts_Heartbeat
class Subscriber
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Subscriber();
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

    /// @brief Receives a status message.
    /// @throws std::invalid_argument if the message cannot be serialized.
    /// @throws std::runtime_error if \c isIinitialized() is false.
    [[nodiscard]] std::unique_ptr<Status> receive() const;

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
