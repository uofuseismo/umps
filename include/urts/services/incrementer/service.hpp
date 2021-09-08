#ifndef URTS_SERVICES_INCREMENTER_SERVICE_HPP
#define URTS_SERVICES_INCREMENTER_SERVICE_HPP
#include <memory>
namespace URTS::Services::Incrementer
{
class Parameters;
/// @class Service service.hpp "urts/services/incrementer/service.hpp"
/// @brief Implements the server-side incrementer service.
/// @copyright Ben Baker (Univeristy of Utah) distributed under the MIT license.
class Service
{
public:
    /// @brief Constructor.
    Service();
    /// @brief Move constructor.
    Service(Service &&service) noexcept;
    /// @brief Move assignment operator.
    Service& operator=(Service &&service) noexcept;
     
    /// @brief Initializes the service.
    void initialize(const Parameters &parameters);
    /// @result True indicates that the service is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if the class is not initialized.
    [[nodiscard]] std::string getName() const;
    /// @brief Gets the address to submit requests to this service.
    /// @throws std::runtime_error if the class is not running.
    [[nodiscard]] std::string getRequestAddress() const;

    /// @brief Starts the service.
    /// @note This would be run something like:
    ///       Service thisService;
    ///       std::thread countingServiceThread(&Service::start, &thisService);
    ///       .
    ///       .
    ///       .
    ///       thisServiceThread.stop(); // Call by main thread
    ///       thisServiceThread.join();
    void start();

    /// @result True indicates that the service is running.
    [[nodiscard]] bool isRunning() const noexcept;

    /// @brief Stops the service.
    void stop();

    /// @brief Destructor.
    ~Service();
 
    Service(const Service &service) = delete;
    Service& operator=(const Service &service) = delete;
private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl;
};
}
#endif
