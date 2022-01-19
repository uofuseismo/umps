#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SERVICE_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SERVICE_HPP
#include <memory>
#include "umps/services/service.hpp"
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Services
 {
  class IService;
 }
 namespace Broadcasts
 {
  class IBroadcast;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
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
    /// @brief Constructor with a given context.
    //explicit Service(std::shared_ptr<zmq::context_t> &context);
    /// @brief Constructor with a given context and logger.
    //Service(std::shared_ptr<zmq::context_t> &context,
    //        std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context and authenticator.
    //Service(std::shared_ptr<zmq::context_t> &context,
    //        std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given logger and authenticator.
    Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
            std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructor with a given context, logger, and authenticator.
    //Service(std::shared_ptr<zmq::context_t> &context,
    //        std::shared_ptr<UMPS::Logging::ILog> &logger,
    //        std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);

    /// @brief Move constructor.
    //Service(Service &&service) noexcept;
    /// @brief Move assignment operator.
    //Service& operator=(Service &&service) noexcept;

    /// @brief Initializes the service.
    void initialize(const Parameters &parameters);

    /// @name Available Connections 
    /// @{
    /// @brief Adds a service connection. 
    /// @param[in] service  The service whose connection information will
    ///                     be added to the connections.
    /// @throws std::invalid_argument if the service's connection information
    ///         cannot be extracted or if the connection already exists.
    /// @throws std::runtime_error if the class is not initialized.
    void addConnection(const UMPS::Services::IService &service);
    /// @brief Adds a broadcast connection.
    /// @param[in] broadcast  The broadcast whose connection information will.
    /// @throws std::runtime_error if the class is not initialized.
    void addConnection(const UMPS::Broadcasts::IBroadcast &broadcast);
    /// @brief Adds a connection.
    /// @param[in] connectionDetails  Contains the name of the connection and the
    ///                               connection information.
    /// @throws std::invalid_argument if the connection information or
    ///         name is undefined or the service is already set.
    /// @sa \c haveConnection()
    /// @throws std::runtime_error if the class is not initialized.
    void addConnection(const Details &connectionDetails);
    /// @brief Removes a connection.
    /// @param[in] name  The name of the connection.
    /// @throws std::runtime_error if \c haveConnection() is false.
    /// @throws std::runtime_error if the class is not initialized.
    void removeConnection(const std::string &name);
    /// @result True indicates that the connection exists.
    [[nodiscard]] bool haveConnection(const std::string &name) const noexcept;
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
 
    /// @brief Starts the service and authenticator.
    /// @throws std::invalid_argument if \c isInitialized() is false.
    void start() final override;
    /// @result True indicates the service was started and is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Stops the service and authenticator.
    void stop() final override;

    /// @brief Destructors.
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
