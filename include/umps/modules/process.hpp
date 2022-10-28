#ifndef UMPS_MODULES_PROCESS_HPP
#define UMPS_MODULES_PROCESS_HPP
#include <functional>
#include <memory>
namespace UMPS::Modules
{
/// @class IProcess "process.hpp" "umps/modules/process.hpp"
/// @brief A module is typically comprised of multiple processes.  This defines
///        the essential qualities that constitute a process in UMPS.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Modules_ProcessesBaseClass
class IProcess
{
public:
    /// @brief Constructor.
    IProcess();

    /// @result The name of the process.
    [[nodiscard]] virtual std::string getName() const noexcept;
    /// @brief Starts the process. 
    /// @note By default this just calls start.
    virtual void operator()();
    /// @brief Starts the process.
    virtual void start() = 0;
    /// @brief Stops the process.
    virtual void stop() = 0;
    /// @result True indicates the process is running.
    [[nodiscard]] virtual bool isRunning() const noexcept = 0;

    /// @brief Sets the stop callback if this process needs to stop the program.
    void setStopCallback(const std::function<void ()> &callback);
    /// @brief Issues the stop command by calling the callback.
    /// @note By default this will do nothing unless \c setStopCallback()
    ///       was called.
    void issueStopCommand();

    /// @brief Destructor.
    virtual ~IProcess();
private:
    class IProcessImpl;
    std::unique_ptr<IProcessImpl> pImpl;
};
}
#endif
