#ifndef UMPS_BROADCASTS_HEARTBEAT_SUBSCRIBER_HPP
#define UMPS_BROADCASTS_HEARTBEAT_SUBSCRIBER_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Broadcasts::Heartbeat
 {
  class Status; 
  class SubscriberOptions;
 }
}
namespace UMPS::Broadcasts::Heartbeat
{
/// @class Subscriber "subscriber.hpp" "umps/broadcasts/heartbeat/subscriber.hpp"
/// @brief A convenience function to subscribe to a heartbeat broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Subscriber
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Subscriber();
    /// @}

    /// @brief Sets the subscriber options.
    /// @throws std::invalid_argument if options.haveAddress() is false.
    /// @throws std::runtime_error if the connection cannot be established.
    void initialize(const SubscriberOptions &options);
    /// @result True indicates that the subscriber is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The security level of the connection.
    [[nodiscard]] Messaging::Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result The socket endpoint.
    /// @throws std::runtime_error if \c isInitialized() is true.
    [[nodiscard]] std::string getEndPoint() const;

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
