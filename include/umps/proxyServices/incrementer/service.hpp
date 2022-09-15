#ifndef UMPS_PROXYSERVICES_INCREMENTER_SERVICE_HPP
#define UMPS_PROXYSERVICES_INCREMENTER_SERVICE_HPP
#include <memory>
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
}
namespace UMPS::ProxyServices::Incrementer
{
class Options;
/// @class Service service.hpp "umps/services/incrementer/service.hpp"
/// @brief Implements the incrementer service.
/// @copyright Ben Baker (Univeristy of Utah) distributed under the MIT license.
class Service
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Service();
    /// @brief Constructor with a given logger.
    explicit Service(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context and a stdout logger.
    explicit Service(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given logger and context.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}
     
    /// @brief Initializes the service.
    void initialize(const Options &options);
    /// @result True indicates that the service is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if the class is not initialized.
    //[[nodiscard]] std::string getName() const;
    /// @brief Gets the address to submit requests to this service.
    /// @throws std::runtime_error if the class is not running.
    //[[nodiscard]] std::string getRequestAddress() const;
    /// @result The details for connecting to the service.
    //[[nodiscard]] ConnectionInformation::Details getConnectionDetails() const;

    /// @brief Starts the service.
    void start();

    /// @result True indicates that the service is running.
    [[nodiscard]] bool isRunning() const noexcept;

    /// @brief Stops the service.
    void stop();

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Service();
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
