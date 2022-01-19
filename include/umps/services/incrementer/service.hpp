#ifndef UMPS_SERVICES_INCREMENTER_SERVICE_HPP
#define UMPS_SERVICES_INCREMENTER_SERVICE_HPP
#include <memory>
#include "umps/services/service.hpp"
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::Services::Incrementer
{
class Parameters;
/// @class Service service.hpp "umps/services/incrementer/service.hpp"
/// @brief Implements the server-side incrementer service.
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
    /// @brief Constructor with a given logger and authenticator.
    Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
            std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Move constructor.
    //Service(Service &&service) noexcept;
    /// @brief Move assignment operator.
    //Service& operator=(Service &&service) noexcept;
    /// @}
     
    /// @brief Initializes the service.
    void initialize(const Parameters &parameters);
    /// @result True indicates that the service is initialized.
    [[nodiscard]] bool isInitialized() const noexcept override final;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if the class is not initialized.
    [[nodiscard]] std::string getName() const override final;
    /// @brief Gets the address to submit requests to this service.
    /// @throws std::runtime_error if the class is not running.
    [[nodiscard]] std::string getRequestAddress() const override final;
    /// @result The details for connecting to the service.
    [[nodiscard]] ConnectionInformation::Details getConnectionDetails() const override final;

    /// @brief Starts the service.
    /// @note This would be run something like:
    ///       Service thisService;
    ///       std::thread countingServiceThread(&Service::start, &thisService);
    ///       .
    ///       .
    ///       .
    ///       thisServiceThread.stop(); // Called by main thread
    ///       countingServiceThread.join();
    void start() override final;

    /// @result True indicates that the service is running.
    [[nodiscard]] bool isRunning() const noexcept;

    /// @brief Stops the service.
    void stop() override final;

    /// @brief Destructor.
    ~Service() override;
 
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
