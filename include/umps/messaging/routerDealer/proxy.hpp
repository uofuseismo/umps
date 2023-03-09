#ifndef UMPS_MESSAGING_ROUTERDEALER_PROXY_HPP
#define UMPS_MESSAGING_ROUTERDEALER_PROXY_HPP
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
  namespace RouterDealer
  {
   class ProxyOptions;
  }
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Proxy;
 }
}
namespace UMPS::Messaging::RouterDealer
{
/// @class Proxy "proxy.hpp" "umps/messaging/routerDealer/proxy.hpp"
/// @brief A ZeroMQ proxy to be used in the ROUTER/DEALER pattern.
/// @details The ROUTER/DEALER pattern is an asynchronous extension of the
///          REQUEST/REPLY pattern.  An intermediary ROUTER/DEALER sits
///          between clients (requesters) and servers (repliers).  The 
///          ROUTER/DEALER will forward requests from clients to servers to
///          process.  The servers return the result to the ROUTER/DEALER
///          which then forwards the reply to the approprirate client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup MessagingPatterns_ReqRep_RouterDealer
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
    /// @brief Constructs a proxy with the given context and logger.
    Proxy(std::shared_ptr<UMPS::Messaging::Context> &context,
          std::shared_ptr<UMPS::Logging::ILog> &logger); 
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes the proxy.
    /// @param[in] options   The options must contain the frontend and backend
    ///                      addresses.  Additionally, options will contain the
    ///                      security protocol.
    /// @throws std::invalid_argument the frontend or backend address is not
    ///         specified. 
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
    //void pause();
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
