#ifndef URTS_MODULES_INCREMENTER_SERVICE_HPP
#define URTS_MODULES_INCREMENTER_SERVICE_HPP
#include <memory>
namespace URTS::Modules::Incrementer
{
class Parameters;
/// @brief Implements the server-side incrementer service.
class Service
{
public:
    Service();
     
    /// @brief Initializes the service.
    void initialize(const Parameters &parameters);
    /// @result True indicates that the service is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @brief Gets the name of the item being incremented.
    /// @throws std::runtime_error if
    [[nodiscard]] std::string getName() const;

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
