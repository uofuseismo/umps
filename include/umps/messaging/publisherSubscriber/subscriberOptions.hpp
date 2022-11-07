#ifndef UMPS_MESSAGING_PUBLISHER_SUBSCRIBER_SUBSCRIBER_OPTIONS_HPP
#define UMPS_MESSAGING_PUBLISHER_SUBSCRIBER_SUBSCRIBER_OPTIONS_HPP
#include <memory>
#include <string>
#include <chrono>
namespace UMPS::MessageFormats
{
 class IMessage;
 class Messages;
}
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Messaging::PublisherSubscriber
{
/// @class SubscriberOptions "publisherOptions.hpp" "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
/// @brief Options for initializing the subscriber in the PUB/SUB pattern.
/// @details The subscriber receives content from a publisher.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_PubSub_PubSub
class SubscriberOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    SubscriberOptions();
    /// @brief Copy constructor.
    /// @param[in] options 
    SubscriberOptions(const SubscriberOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The class from which to initialize this class.
    ///                         On exit, options's behavior is undefined.  
    SubscriberOptions(SubscriberOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options to copy to this.
    /// @result A deep copy of the input options.
    SubscriberOptions& operator=(const SubscriberOptions &options);
    /// @brief Move assignment operators.
    /// @param[in,out] options  The options whose memory will be moved to this.
    ///                         On exit, options's behavior is undefined.
    /// @result The memory moved from options to this.
    SubscriberOptions& operator=(SubscriberOptions &&options) noexcept;
    /// @}

    /// @name Subscriber Options
    /// @{

    /// @brief Sets the address to which the subscriber will bind.
    /// @param[in] address  The address to which the subscriber will bind.
    /// @throws std::invalid_argument if the address is empty.
    void setAddress(const std::string &address);
    /// @result The socket's address.
    /// @throws std::invalid_argument if \c haveAddress is false. 
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;

    /// @brief Sets the message types to which to subscriber.
    /// @param[in] messageTypes   The message types that the subscriber will
    ///                           be able to read and deserialize.
    void setMessageTypes(const UMPS::MessageFormats::Messages &messageTypes);
    /// @result The message types to which to subscriber.
    /// @throws std::runtime_error if \c haveMessageTypes() is false.
    [[nodiscard]] UMPS::MessageFormats::Messages getMessageTypes() const;
    /// @result True indicates the message types have been set.
    [[nodiscard]] bool haveMessageTypes() const noexcept;

    /// @brief Influences the number of messages that can be queued on the
    ///        socket.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setReceiveHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getReceiveHighWaterMark() const noexcept;

    /// @brief If the subscriber waits the timeOut length of time before
    ///        receiving a message then it will return without a message.
    ///        This is useful when the subscriber thread has other
    ///        responsibilities.
    /// @param[in] timeOut   The time out duration in milliseconds.  If this is
    ///                      zero then the subscriber will immediately return.
    ///                      If this is negative then the subscriber will wait
    ///                      indefinitely until a message is received.
    void setReceiveTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The time out duration in milliseconds.
    [[nodiscard]] std::chrono::milliseconds getReceiveTimeOut() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol Options
    /// @{

    /// @brief Defines the ZAP options to be used when configuring the socket.
    /// @param[in] options  The ZAP options.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.  By default this will configure sockets with
    ///         the grasslands (no security) pattern.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}
 
    /// @name Destructors
    /// @{

    /// @brief Resets class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~SubscriberOptions();
    /// @}
private:
    class SubscriberOptionsImpl;
    std::unique_ptr<SubscriberOptionsImpl> pImpl;
};
}
#endif
