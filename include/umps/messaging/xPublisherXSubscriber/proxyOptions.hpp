#ifndef UMPS_XPUBLISHERXSUBSCRIBER_PROXYOPTIONS_HPP
#define UMPS_XPUBLISHERXSUBSCRIBER_PROXYOPTIONS_HPP
#include <memory>
#include <string>
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Messaging::XPublisherXSubscriber
{
/// @class ProxyOptions "proxyOptions.hpp" "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
/// @brief Options for initializing the proxy.
/// @details A proxy is an intermediary that allows for an asynchronous pub/sub
///          pattern.  In this case, there may be multiple publishers sending
///          messages to this proxy.  The subscribers then read all of these
///          messages of desired type from the proxy without having to know
///          all of the individual publisher connection information. 
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class ProxyOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ProxyOptions();
    /// @brief Copy constructor.
    /// @param[in] options 
    ProxyOptions(const ProxyOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The class from which to initialize this class.
    ///                         On exit, options's behavior is undefined.  
    ProxyOptions(ProxyOptions &&options) noexcept;

    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options to copy to this.
    /// @result A deep copy of the input options.
    ProxyOptions& operator=(const ProxyOptions &options);
    /// @brief Move assignment operators.
    /// @param[in,out] options  The options whose memory will be moved to this.
    ///                         On exit, options's behavior is undefined.
    /// @result The memory moved from options to this.
    ProxyOptions& operator=(ProxyOptions &&options) noexcept;

    /// @}

    /// @name Publisher Options
    /// @{

    /// @brief Sets the address to which publishers connect.
    /// @throws std::invalid_argument if the address is empty.
    void setFrontendAddress(const std::string &address);
    /// @result The frontend address.
    /// @throws std::invalid_argument if \c haveFrontendAddress is false. 
    [[nodiscard]] std::string getFrontendAddress() const;
    /// @result True indicates the frontend address was set.
    [[nodiscard]] bool haveFrontendAddress() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued by the frontend.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setFrontendHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getFrontendHighWaterMark() const noexcept;

    /// @}

    /// @name Subscriber Options
    /// @{

    /// @brief Sets the address to which subscribers connect.
    /// @param[in] address  The address to which subscribers will connect.
    /// @throws std::invalid_argument if the address is empty.
    void setBackendAddress(const std::string &address);
    /// @result The backend address.
    /// @throws std::invalid_argument if \c haveBackendAddress is false. 
    [[nodiscard]] std::string getBackendAddress() const;
    /// @result True indicates the backend address was set.
    [[nodiscard]] bool haveBackendAddress() const noexcept;

    /// @brief This sets a hard limit on the maximum number of messages that
    ///        can be queued by the backend.
    /// @param[in] highWaterMark  The high limit on the maximum number of
    ///                           messages that this socket can queue.
    ///                           0 sets this to "infinite."  
    /// @throws std::invalid_argument if the high water mark is negative. 
    void setBackendHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0 (infinite).
    [[nodiscard]] int getBackendHighWaterMark() const noexcept;

    /// @}
 
    /// @name Topic
    /// @{

    /// @brief Sets the topic for inprocess communication.  The control
    ///        will talk to the service via "inproc://" + topic + "_control"
    ///        hence topic should be a unique name such as the broadcast name.
    /// @param[in] topic   The topic.
    /// @throws std::invalid_argument if topic is blank.
    void setTopic(const std::string &topic);
    /// @result The topic name.
    /// @throws std::runtime_error if \c haveTopic() is false.
    [[nodiscard]] std::string getTopic() const;
    /// @result True indicates that the topic was set.
    [[nodiscard]] bool haveTopic() const noexcept;

    /// @}

    /// @name ZeroMQ Authentication Protocol Options
    /// @{

    /// @brief Defines the ZAP options to be used when configuring the socket.
    /// @param[in] options  The ZAP options.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.  By default this will configure sockets with
    ///         the grasslands (no security) pattern.
    [[nodiscard]] Authentication::ZAPOptions getZAPOptions() const noexcept;

    /// @}
 
    /// @name Destructors
    /// @{

    /// @brief Resets class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~ProxyOptions();

    /// @}
private:
    class ProxyOptionsImpl;
    std::unique_ptr<ProxyOptionsImpl> pImpl;
};
}
#endif
