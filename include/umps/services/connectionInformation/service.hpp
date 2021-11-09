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

    /// @brief Initializes the service.
    void initialize(const Parameters &parameters);

    /// @name Available Services
    /// @{
    void addService(std::shared_ptr<const UMPS::Services::IService> &service);
    /// @brief Adds a service.
    /// @param[in] serviceDetails  Contains the name of the service and the
    ///                            connection information.
    /// @throws std::invalid_argument if the connection information or service
    ///         name is undefined or the service is already set.
    /// @sa \c haveService()
    void addService(const Details &serviceDetails);
    /// @brief Removes a service.
    /// @param[in] name  The name of the service.
    /// @throws std::runtime_error if \c haveService() is false.
    void removeService(const std::string &name);
    /// @param[in] name  The name of the service.
    /// @result True indicates that the service exists.
    [[nodiscard]] bool haveService(const std::string &name) const noexcept;
    /// @}

    /// @name Available Broadcasts
    /// @{
    /// @brief Adds a broadcast.
    /// @param[in] broadcastDetails  Contains the name of the broadcast and the
    ///                              connection information.
    /// @throws std::invalid_argument if the connection information or
    ///         broadcast name is undefined or the broadcast is already set.
    /// @sa \c haveBroadcast()
    void addBroadcast(const Details &broadcastDetails);
    /// @brief Removes a broadcast.
    /// @param[in] name  The name of the broadcast.
    /// @throws std::runtime_error if \c haveBroadcast() is false.
    void removeBroadcast(const std::string &name);
    /// @param[in] name  The name of the broadcast.
    /// @result True indicates that the broadcast exists.
    [[nodiscard]] bool haveBroadcast(const std::string &name) const noexcept;
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
    [[nodiscard]] Details getConnectionDetails() const override final;
 
    /// @brief Starts the service.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    void start() final override;
    /// @result True indicates the service was started and is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Stops the service.
    void stop() final override;

    /// @brief Destructors.
    virtual ~Service();

    Service(const Service &service) = delete;
    Service& operator=(const Service &service) = delete;
private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl;
};
}
#endif
