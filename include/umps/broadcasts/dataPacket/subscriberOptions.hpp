#ifndef UMPS_BROADCASTS_DATAPACKET_SUBSCRIBEROPTIONS_HPP
#define UMPS_BROADCASTS_DATAPACKET_SUBSCRIBEROPTIONS_HPP
#include <memory>
#include <chrono>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
 namespace Messaging
 {
  namespace PublisherSubscriber
  {
   class SubscriberOptions;
  }
 }
}
namespace UMPS::Broadcasts::DataPacket
{
/// @class SubscriberOptions "subscriberOptions.hpp" "umps/broadcasts/dataPacket/subscriberOptions.hpp"
/// @brief Defines the parameters for connecting to the data packet backend.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
class SubscriberOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    SubscriberOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    SubscriberOptions(const SubscriberOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize this
    ///                         class.  On exit, options's behavior is
    ///                         undefined.
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

    /// @name Required Parameters
    /// @{

    /// @brief Sets the address to which the proxy will bind.
    /// @throws std::invalid_argument if the address is empty.
    void setAddress(const std::string &address);
    /// @result The socket's address.
    /// @throws std::invalid_argument if \c haveAddress is false. 
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name Options Parameters
    /// @{

    /// @brief Defines the ZAP options to be used when configuring the socket.
    /// @param[in] options  The ZAP options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    /// @result The ZAP options.  By default this will configure sockets with
    ///         the grasslands (no security) pattern.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getHighWaterMark() const noexcept;

    /// @brief If the subscriber waits the timeOut length of time before
    ///        receiving a message then it will return without a message.
    ///        This is useful when the subscriber thread has other
    ///        responsibilities.
    /// @param[in] timeOut   The time out duration in milliseconds.  If this is
    ///                      zero then the subscriber will immediately return.
    ///                      If this is negative then the subscriber will wait
    ///                      indefinitely until a message is received.
    void setTimeOut(const std::chrono::milliseconds timeOut) noexcept;
    /// @result The time out duration in milliseconds.
    [[nodiscard]] std::chrono::milliseconds getTimeOut() const noexcept;
    /// @}

    /// @result The subscriber options.
    [[nodiscard]] UMPS::Messaging::PublisherSubscriber::SubscriberOptions
        getSubscriberOptions() const noexcept;

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
