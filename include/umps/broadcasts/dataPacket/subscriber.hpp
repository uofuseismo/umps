#ifndef UMPS_BROADCASTS_DATAPACKET_SUBSCRIBER_HPP
#define UMPS_BROADCASTS_DATAPACKET_SUBSCRIBER_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
  template<class T> class DataPacket;
 }
 namespace Broadcasts::DataPacket
 {
  template<class T> class SubscriberOptions;
 }
}
namespace UMPS::Broadcasts::DataPacket
{
/// @class Subscriber "subscriber.hpp" "umps/broadcasts/dataPacket/subscriber.hpp"
/// @brief A convenience function to subscribe to a dataPacket broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
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
    void initialize(const SubscriberOptions<T> &options);
    /// @result True indicates that the subscriber is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The security level of the connection.
    [[nodiscard]] Messaging::Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result The socket endpoint.
    /// @throws std::runtime_error if \c isInitialized() is true.
    [[nodiscard]] std::string getEndPoint() const;

    /// @brief Receives a data packet message.
    /// @throws std::invalid_argument if the message cannot be serialized.
    /// @throws std::runtime_error if \c isIinitialized() is false.
    [[nodiscard]] std::unique_ptr<MessageFormats::DataPacket<T>> receive() const;

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
