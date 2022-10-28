#ifndef UMPS_SERVICES_COMMAND_SERVICE_HPP
#define UMPS_SERVICES_COMMAND_SERVICE_HPP
#include <memory>
#include "umps/services/service.hpp"
namespace UMPS
{
 namespace Messaging
 {
  class Context;
  class IMessage;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Services
 {
  namespace ConnectionInformation
  {
   class Details;
  }
  namespace Command
  {
   class ServiceOptions;
  }
 }
}
namespace UMPS::Services::Command
{
/// @class Service "service.hpp" "umps/services/command/service.hpp"
/// @brief This class allows a background application to interact with a user.
/// @details This class will write the IPC file details for this module to the
///          local modules table.  A user's application can query this table
///          for the IPC file then directly make requests to the application
///          via the underlying request-router.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Service : public UMPS::Services::IService
{
public:
    /// @name Constructors
    /// @{
 
    /// @brief Constructor.
    Service();
    /// @brief Constructor with a logger.
    explicit Service(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a context.
    explicit Service(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a context and logger
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Sets the local service options.
    void initialize(const ServiceOptions &options);
    /// @}

    /// @result True indicates that the service is initialized.
    [[nodiscard]] bool isInitialized() const noexcept final;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if the class is not initialized.
    [[nodiscard]] std::string getName() const final;
    /// @brief Gets the address to submit requests to this service.
    /// @throws std::runtime_error if the class is not running.
    [[nodiscard]] std::string getRequestAddress() const final;
    /// @result The connection details for connecting to the service.
    [[nodiscard]] ConnectionInformation::Details getConnectionDetails() const final;

    /// @brief Starts the service and authenticator.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    void start() final;
    /// @result True indicates the service was started and is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Stops the service and authenticator.
    void stop() final;

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Service() override;
    /// @}

    Service(const Service &) = delete;
    Service(Service &&) noexcept = delete;
    Service& operator=(const Service &) = delete;
    Service& operator=(Service &&) noexcept = delete;
private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl; 
};
}
#endif
