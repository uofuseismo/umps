#ifndef UMPS_SERVICES_COMMAND_REMOTE_PROXY_HPP
#define UMPS_SERVICES_COMMAND_REMOTE_PROXY_HPP
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
  namespace Command
  {
   class RemoteProxyOptions;
  }
  namespace ConnectionInformation
  { 
   class Details;
  }
 }
}
namespace UMPS::Services::Command
{
class RemoteProxy
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RemoteProxy();
    /// @brief Constructor with a given logger.
    explicit RemoteProxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a logger and given authenticator.
    RemoteProxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
                std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given logger and frontend and backend authenticator.
    RemoteProxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
                std::shared_ptr<UMPS::Authentication::IAuthenticator> &frontendAuthenticator,
                std::shared_ptr<UMPS::Authentication::IAuthenticator> &backendAuthenticator);
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes the proxy.
    /// @throws std::invalid_argument if the frontend and backend address are not set.
    void initialize(const RemoteProxyOptions &options);
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
    ~RemoteProxy();
    /// @}
private:
    class RemoteProxyImpl;
    std::unique_ptr<RemoteProxyImpl> pImpl;
};
}
#endif
