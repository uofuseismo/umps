#ifndef UMPS_MODULES_PROCESSMANAGER_HPP
#define UMPS_MODULES_PROCESSMANAGER_HPP
#include <memory>
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Modules
 {
  class IProcess;
 }
}
namespace UMPS::Modules
{
/// @class ProcessManager "processManager.hpp" "umps/modules/processManager.hpp"
/// @brief This is a collection of processes.  By adding processes to this
///        module, all processes can be started and stopped by one utility.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Modules_ProcessesBaseClass
class ProcessManager
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ProcessManager();
    explicit ProcessManager(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @brief Adds a process to the manager.
    /// @param[in,out] process  The process to bring under process management.
    ///                         Note, this class takes ownership or process.
    ///                         Therefore, processes's behavior on exit is
    ///                         undefined.
    /// @throws std::invalid_argument if exists(process) is true;
    void insert(std::unique_ptr<IProcess> &&process);
    /// @result True indicates the process exists.
    /// @throws std::invalid_argument if process.getName() is not set.
    [[nodiscard]] bool contains(const IProcess &process) const noexcept;
    /// @result True idicates the process exists.
    [[nodiscard]] bool contains(const std::string &name) const noexcept;

    /// @brief Attempts to start all processes.
    void start();
    /// @result True indicates the processes are running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Attempts to stop all processes.
    void stop();

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~ProcessManager();
    /// @}

    ProcessManager(const ProcessManager &) = delete;
    ProcessManager& operator=(const ProcessManager &) = delete;
private:
    class ProcessManagerImpl;
    std::unique_ptr<ProcessManagerImpl> pImpl;
};
}
#endif
