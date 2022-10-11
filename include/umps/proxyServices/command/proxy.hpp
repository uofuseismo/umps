#ifndef UMPS_PROXY_SERVICES_COMMAND_PROXY_HPP
#define UMPS_PROXY_SERVICES_COMMAND_PROXY_HPP
#include <memory>
namespace UMPS
{
 namespace Authentication
 {
  class IAuthenticator;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
 namespace Services
 {
  namespace ConnectionInformation
  { 
   class Details;
  }
 }
 namespace ProxyServices::Command
 {
  class ProxyOptions;
 }
}
/// @class Proxy "proxy.hpp" "umps/proxyServices/command/proxy.hpp"
/// @brief This is the intermediary that allows communication between a client
///        and a backend service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
namespace UMPS::ProxyServices::Command
{
class Proxy
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Proxy();
    /// @brief Constructor with a given logger.
    explicit Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a logger and given authenticator.
    Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
                std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given logger and frontend and backend authenticator.
    Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
                std::shared_ptr<UMPS::Authentication::IAuthenticator> &frontendAuthenticator,
                std::shared_ptr<UMPS::Authentication::IAuthenticator> &backendAuthenticator);
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes the proxy.
    /// @throws std::invalid_argument if the frontend and backend address are not set.
    void initialize(const ProxyOptions &options);
    /// @result True indicates the class is initialized.
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
    /// @note This will spin off a thread so the calling program must
    ///       call \c stop().
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
private:
    class ProxyImpl;
    std::unique_ptr<ProxyImpl> pImpl;
};
}
#endif
