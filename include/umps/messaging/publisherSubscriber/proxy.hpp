#ifndef UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PROXY_HPP
#define UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PROXY_HPP
#include <memory>
#include <string>
#include "umps/messaging/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  namespace PublisherSubscriber
  {
   class ProxyOptions;
  }
  namespace Authentication::Certificate
  {
   class Keys;
   class UserNameAndPassword;
  }
 }
}
namespace zmq 
{
 class context_t;
}
namespace UMPS::Messaging::PublisherSubscriber
{
/// @class Proxy "proxy.hpp" "umps/messaging/publisherSubscriber/proxy.hpp"
/// @brief A ZeroMQ proxy to be used in the XPUB/XSUB pattern.
/// @detail The XPUB/XSUB pattern is an extension of the PUB/SUB pattern.
///         The proxy is an intermediary which can forward messages from 
///         multiple publishers to multiple subscribers.   Conceptually,
///         data flows from the frontend to the backend.  Hence, for a 
///         pub/sub pattern publishers will connect to the frontend and
///         subscribers will connect to the backend.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Proxy
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Proxy();
    /// @brief Constructs a publisher with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] proxy  The proxy from which to initialize this class.
    ///                       On exit, proxy's behavior is undefined.
    Proxy(Proxy &&proxy) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment.
    /// @param[in,out] proxy  The proxy whose memory will be moved to this.
    ///                       On exit, proxy's behavior is undefined.
    /// @result The memory from proxy moved to this.
    Proxy& operator=(Proxy &&proxy) noexcept;
    /// @}

    /// @name Initialization
    /// @{
    /// @brief Initializes the proxy.
    /// @param[in] options   The options must contain the frontend and backend
    ///                      addresses as well as the proxy's topic.
    ///                      Additionally, options will contain the security
    ///                      protocol.
    /// @throws std::invalid_argument the frontend or backend address is not
    ///         specified or the topic is not specified.
    /// @throws std::runtime_error if the creation of the proxy fails.
    void initialize(const ProxyOptions &options);
    /// @result The security level of the connection.
    [[nodiscard]] Authentication::SecurityLevel getSecurityLevel() const noexcept;

    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The address to which the consumers will subscribe.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    [[nodiscard]] std::string getFrontendAddress() const;
    /// @result The address to which the producers will subscribe.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    [[nodiscard]] std::string getBackendAddress() const;
    /// @}

    /// @name Usage
    /// @{
    /// @brief Starts the proxy.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start(); 
    /// @result True indicates the proxy was started and is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Pauses the proxy.
    /// @note You can restart the proxy by using \c start().
    /// @throws std::runtime_error if \c isInitialized() is false.
    void pause();
    /// @brief Stops the proxy.
    /// @note You will have to reinitialize after calling this.
    void stop();
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Destructor.
    ~Proxy();
    /// @}

    Proxy(const Proxy &proxy) = delete;
    Proxy& operator=(const Proxy &proxy) = delete;
private:
    class ProxyImpl;
    std::unique_ptr<ProxyImpl> pImpl;
};
}
#endif
