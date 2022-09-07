#ifndef UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHEROPTIONS_HPP
#define UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHEROPTIONS_HPP
#include <memory>
#include <string>
#include <chrono>
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Messaging::PublisherSubscriber
{
/// @class PublisherOptions "publisherOptions.hpp" "umps/messaging/publisherSubscriber/publisherOptions.hpp"
/// @brief Options for initializing the publisher in the PUB/SUB pattern.
/// @details The publisher sends content and it is up to the subscriber to
///          be listening.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_PubSub_PubSub
class PublisherOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    PublisherOptions();
    /// @brief Copy constructor.
    /// @param[in] options 
    PublisherOptions(const PublisherOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The class from which to initialize this class.
    ///                         On exit, options's behavior is undefined.  
    PublisherOptions(PublisherOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options to copy to this.
    /// @result A deep copy of the input options.
    PublisherOptions& operator=(const PublisherOptions &options);
    /// @brief Move assignment operators.
    /// @param[in,out] options  The options whose memory will be moved to this.
    ///                         On exit, options's behavior is undefined.
    /// @result The memory moved from options to this.
    PublisherOptions& operator=(PublisherOptions &&options) noexcept;
    /// @}

    /// @name Publisher Options
    /// @{

    /// @brief Sets the address to which the publisher will bind.
    /// @param[in] address  The address to which the publisher will connect.
    /// @throws std::invalid_argument if the address is empty.
    void setAddress(const std::string &address);
    /// @result The socket's address.
    /// @throws std::invalid_argument if \c haveAddress is false. 
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getHighWaterMark() const noexcept;

    /// @brief If the publisher waits the timeOut length of time before
    ///        sending a message then it will without having sent the message.
    ///        This is useful when the publisher thread has other
    ///        responsibilities.
    /// @param[in] timeOut   The time out duration in milliseconds.  If this is
    ///                      zero then the subscriber will immediately return.
    ///                      If this is negative then the subscriber will wait
    ///                      indefinitely until a message is received.
    void setTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    /// @result The time out duration in milliseconds.
    [[nodiscard]] std::chrono::milliseconds getTimeOut() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol Options
    /// @{

    /// @brief Defines the ZAP options to be used when configuring the socket.
    /// @param[in] options  The ZAP options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    /// @result The ZAP options.  By default this will configure sockets with
    ///         the grasslands (no security) pattern.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}
 
    /// @name Destructors
    /// @{

    /// @brief Resets class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~PublisherOptions();
    /// @}
private:
    class PublisherOptionsImpl;
    std::unique_ptr<PublisherOptionsImpl> pImpl;
};
}
#endif
