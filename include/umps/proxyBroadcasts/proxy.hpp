#ifndef UMPS_PROXYBROADCASTS_PROXY_HPP
#define UMPS_PROXYBROADCASTS_PROXY_HPP
#include <memory>
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Services::ConnectionInformation
 {
  class Details;
 }
 namespace ProxyBroadcasts
 {
  class ProxyOptions;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::ProxyBroadcasts
{
/// @class Proxy "proxy.hpp" "umps/proxyBroadcasts/proxy.hpp"
/// @brief This defines the XPUB/XSUB proxy to data broadcasts.
///        This is an intermediate message layer to which producers
///        can publish messages to this XSUB socket and from which
///        subscribers can receive content from this XPUB socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Proxy
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructs the XPUB/XSUB proxy.
    Proxy();
    /// @brief Constructs the XPUB/XSUB with a given logger.
    explicit Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given logger and authenticator.
    Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
          std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given logger and a frontend and backend
    ///        authenticator.
    Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
          std::shared_ptr<UMPS::Authentication::IAuthenticator> &frontendAuthenticator,
          std::shared_ptr<UMPS::Authentication::IAuthenticator> &backendAuthenticator);
    /// @}

    /// @brief Initializes the proxy.
    /// @param[in] options  The proxy options.  At a minimum, this must 
    ///                     contain the frontend and backend address.
    /// @throws std::invalid_argument if the options are invalid.
    void initialize(const ProxyOptions &options);

    /// @name Initialization
    /// @{

    /// @result True indicates that the proxy is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The name of the proxy broadcast.
    [[nodiscard]] std::string getName() const;
    /// @result The connection details.
    [[nodiscard]] Services::ConnectionInformation::Details getConnectionDetails() const;
    /// @}

    /// @name Starts the proxy.
    /// @{

    /// @brief Starts the proxy.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start();
    /// @result True indicates the proxy is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Stops the proxy.
    void stop();
    /// @}

    /// @result An uninitialized instance of this class.
    //[[nodiscard]] std::unique_ptr<IProxy> createInstance() const noexcept override final;

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
