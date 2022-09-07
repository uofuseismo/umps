#ifndef UMPS_PROXYSERVICES_PROXY_HPP
#define UMPS_PROXYSERVICES_PROXY_HPP
#include <memory>
#include "umps/proxyServices/proxy.hpp"
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
 namespace ProxyServices
 {
  class ProxyOptions;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::ProxyServices
{
/// @class Proxy "proxy.hpp" "umps/proxyServices/proxy.hpp"
/// @brief This defines the ROUTER/DEALER proxy.  This is an intermediary
///        message layer to which (1) clients send requests to the frontend,
///        then the request is (2) propagated to the backend and the servers.  
///        The servers then (3) send a reply to the backend which is then
///        (3) propagated to the frontend, and (4) finally the reply is
///        received by the original client.
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
    //Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
    //      std::shared_ptr<UMPS::Authentication::IAuthenticator> &frontendAuthenticator,
    //      std::shared_ptr<UMPS::Authentication::IAuthenticator> &backendAuthenticator);
    /// @}

    /// @brief Initializes the proxy.
    /// @param[in] options  The proxy options.  At a minimum, this must 
    ///                     contain the frontend and backend address
    ///                     as well as the service's name.
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
