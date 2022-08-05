#ifndef UMPS_SERVICES_MODULEREGISTRY_SERVICE_HPP
#define UMPS_SERVICES_MODULEREGISTRY_SERVICE_HPP
#include <memory>
#include "umps/services/service.hpp"
namespace UMPS
{
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
  namespace ModuleRegistry
  {
   class ModuleDetails;
   class ServiceOptions;
  }
 }
 namespace ProxyBroadcasts
 {
  class Proxy;
 }
 namespace ProxyServices
 { 
  class Proxy;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::Services::ModuleRegistry
{
/// @class Service service.hpp "umps/services/moduleRegistry/service.hpp"
/// @brief Implements the server-side service for registering modules.
/// @copyright Ben Baker (Univeristy of Utah) distributed under the MIT license.
class Service : public UMPS::Services::IService
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Service();
    /// @brief Constructor with a given logger.
    explicit Service(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context.
    explicit Service(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given context and logger.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context and authenticator.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given logger and authenticator.
    Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
            std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given context, logger, and authenticator.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger,
            std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @}

    /// @brief Initializes the service.
    /// @param[in] options  The options controlling the underlying router
    ///                     connection.
    /// @throws std::invalid_argument if options.getClientAccessAddress()
    ///         is not set.
    void initialize(const ServiceOptions &options);

    /// @name Usage
    /// @{

    /// @brief Registers a module.
    /// @param[in] module   The module to register.
    /// @throws std::invalid_argument if the service's connection information
    ///         cannot be extracted or if the connection already exists.
    /// @throws std::runtime_error if the class is not initialized.
    void insert(const ModuleDetails &module);
    
    /// @brief Removes a module.
    /// @param[in] module   The module to remove.
    /// @throws std::invalid_argument if \c contains(module) is false.
    /// @throws std::runtime_error if the class is not initialized.
    void erase(const ModuleDetails &module);
    /// @brief Removes a module.
    /// @param[in] name  The name of the module to remove.
    /// @throws std::invalid_argument if \c contains(name) is false.
    /// @throws std::runtime_error if the class is not initialized.
    void erase(const std::string &name);

    /// @param[in] module  The module details.
    /// @result True indicates that the module exists.
    [[nodiscard]] bool contains(const ModuleDetails &module) const noexcept;
    /// @param[in] name  The name of the module.
    /// @result True indicates that the module exists.
    [[nodiscard]] bool contains(const std::string &name) const noexcept;
    /// @}

    /// @result True indicates that the service is initialized.
    [[nodiscard]] bool isInitialized() const noexcept override final;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if the class is not initialized.
    [[nodiscard]] std::string getName() const override final;
    /// @brief Gets the address to submit requests to this service.
    /// @throws std::runtime_error if the class is not running.
    [[nodiscard]] std::string getRequestAddress() const override final;
    /// @result The connection details for connecting to the service.
    [[nodiscard]] UMPS::Services::ConnectionInformation::Details getConnectionDetails() const override final;
 
    /// @brief Starts the service and authenticator.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    void start() final override;
    /// @result True indicates the service was started and is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Stops the service and authenticator.
    void stop() final override;

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Service() override;
    /// @}

    Service(const Service &service) = delete;
    Service& operator=(const Service &service) = delete;
    Service(Service &&service) noexcept = delete;
    Service& operator=(Service &&service) noexcept = delete;
private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl;
};
}
#endif
