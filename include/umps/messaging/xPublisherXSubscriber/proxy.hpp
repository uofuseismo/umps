#ifndef UMPS_MESSAGING_XPUBLISHER_XSUBSCRIBER_PROXY_HPP
#define UMPS_MESSAGING_XPUBLISHER_XSUBSCRIBER_PROXY_HPP
#include <memory>
#include <string>
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
  namespace XPublisherXSubscriber
  {
   class ProxyOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Proxy;
 }
}
namespace UMPS::Messaging::XPublisherXSubscriber
{
/// @class Proxy "proxy.hpp" "umps/messaging/xPublisherXSubscriber/proxy.hpp"
/// @brief A ZeroMQ proxy to be used in the XPUB/XSUB pattern.
/// @details The XPUB/XSUB pattern is an extension of the PUB/SUB pattern.
///          The proxy is an intermediary which can forward messages from 
///          multiple publishers to multiple subscribers.   Conceptually,
///          data flows from the frontend to the backend.  Hence, for a 
///          pub/sub pattern publishers will connect to the frontend and
///          subscribers will connect to the backend.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_PubSub_XPubXSub
class Proxy
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Proxy();
    /// @brief Constructs a proxy with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a proxy with the given context.
    /// @param[in] context  A pointer to the application's context.
    explicit Proxy(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructs a proxy with a given frontend and backend context.
    /// @param[in] frontendContext  The context for the frontend (where the
    ///                             publishers will connect).
    /// @param[in] backendContext   The context for the backend (where the
    ///                             subscribers will connect).
    Proxy(std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
          std::shared_ptr<UMPS::Messaging::Context> &backendContext);
    /// @brief Constructs a proxy with the given context and logger.
    Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
          std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a proxy with the given contexts and logger.
    Proxy(std::shared_ptr<UMPS::Messaging::Context> &frontendContext,
          std::shared_ptr<UMPS::Messaging::Context> &backendContext,
          std::shared_ptr<UMPS::Logging::ILog> &logger);
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
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Proxy getSocketDetails() const;
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
    Proxy(Proxy &&proxy) noexcept = delete;
    Proxy& operator=(const Proxy &proxy) = delete;
    Proxy& operator=(Proxy &&proxy) noexcept = delete;
private:
    class ProxyImpl;
    std::unique_ptr<ProxyImpl> pImpl;
};
}
#endif
