#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SERVICE_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SERVICE_HPP
#include <memory>
#include "umps/services/service.hpp"
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Services::ConnectionInformation
{
class Parameters;
/// @class Service service.hpp "umps/services/incrementer/service.hpp"
/// @brief Implements the server-side connection information service.
/// @copyright Ben Baker (Univeristy of Utah) distributed under the MIT license.
class Service : public UMPS::Services::IService
{
public:
    /// @brief Constructor.
    Service();
    /// @brief Constructor with a given logger.
    explicit Service(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    Service(Service &&service) noexcept;
    /// @brief Move assignment operator.
    Service& operator=(Service &&service) noexcept;

/*
    /// @brief Initializes the service.
    void initialize(const Parameters &parameters);
    /// @result True indicates that the service is initialized.
*/
    [[nodiscard]] bool isInitialized() const noexcept override final;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if the class is not initialized.
    [[nodiscard]] std::string getName() const override final;
    /// @brief Gets the address to submit requests to this service.
    /// @throws std::runtime_error if the class is not running.
    [[nodiscard]] std::string getRequestAddress() const override final;

private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl;
};
}
#endif
