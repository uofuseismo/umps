#ifndef UMPS_SERVICES_COMMAND_REMOTEPROXY_HPP
#define UMPS_SERVICES_COMMAND_REMOTEPROXY_HPP
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
  class RemoteProxyOptions;
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

    /// @brief Initializes the proxy.
    /// @throws std::invalid_argument if the frontend and backend address are not set.
    void initialize(const RemoteProxyOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;

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
