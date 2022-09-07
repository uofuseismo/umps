#ifndef UMPS_SERVICES_COMMAND_LOCALSERVICE_HPP
#define UMPS_SERVICES_COMMAND_LOCALSERVICE_HPP
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
   class LocalServiceOptions;
  }
 }
}
namespace UMPS::Services::Command
{
/// @class LocalService "localService.hpp" "umps/services/command/localService.hpp"
/// @brief This class allows a background application to interact with a user.
/// @detail This class will write the IPC file details for this module to the
///         local modules table.  A user's application can query this table
///         for the IPC file then directly make requests to the application
///         via the underlying request-router.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class LocalService : public UMPS::Services::IService
{
public:
    /// @name Constructors
    /// @{
 
    /// @brief Constructor.
    LocalService();
    /// @brief Constructor with a logger.
    explicit LocalService(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a context.
    explicit LocalService(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a context and logger
    LocalService(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Sets the local service options.
    void initialize(const LocalServiceOptions &options);
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

    ~LocalService() override;
    /// @}

    LocalService(const LocalService &) = delete;
    LocalService(LocalService &&) noexcept = delete;
    LocalService& operator=(const LocalService &) = delete;
    LocalService& operator=(LocalService &&) noexcept = delete;
private:
    class LocalServiceImpl;
    std::unique_ptr<LocalServiceImpl> pImpl; 
};
}
#endif
