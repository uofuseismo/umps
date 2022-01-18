#ifndef UMPS_SERVICES_SERVICE_HPP
#define UMPS_SERVICES_SERVICE_HPP
#include <memory>
#include <string>
#include "umps/services/connectionInformation/details.hpp"
namespace UMPS::Services
{
/// @class IServer "service.hpp" "umps/services/service.hpp"
/// @brief This is the abstract base class which defines a service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IService
{
public:
    /// @brief Destructor.
    virtual ~IService() = default;
    /// @result True indicates that the publisher is initialized.
    [[nodiscard]] virtual bool isInitialized() const noexcept = 0;
    /// @result The name of the publisher.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] virtual std::string getName() const = 0;
    /// @result The address to which the requestor would submit a request
    ///         to this service.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] virtual std::string getRequestAddress() const = 0;
    /// @result The connection details for connecting to the service.
    [[nodiscard]] virtual ConnectionInformation::Details getConnectionDetails() const = 0;
    /// @brief Starts the service and authenticator service.
    virtual void start() = 0;
    /// @brief Stops the service and authenticator service.
    virtual void stop() = 0;
};
}
#endif
