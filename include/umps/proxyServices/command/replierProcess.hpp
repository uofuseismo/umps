#ifndef UMPS_PROXY_SERVICES_COMMAND_REPLIER_PROCESS_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REPLIER_PROCESS_HPP
#include <memory>
#include <chrono>
#include "umps/modules/process.hpp"
namespace UMPS
{
 namespace Services::ConnectionInformation
 {
  class Requestor;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
 namespace ProxyServices::Command
 {
  class ReplierProcessOptions;
  class Replier;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class "umps/proxyServices/command/replierProcess.hpp"
/// @brief This process will respond to remote module interaction messages.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup UMPS_ProxyServices_Command
class ReplierProcess : public UMPS::Modules::IProcess
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ReplierProcess();
    /// @brief Constructor with a given logger.
    explicit ReplierProcess(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context.
    explicit ReplierProcess(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given logger and context.
    //ReplierProcess(std::shared_ptr<UMPS::Messaging::Context> &context,
    //               std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the process options.
    /// @param[in] options            The replier process options.
    /// @param[in] replierConnection  The connection to the replier.
    /// @throws std::invalid_argument if \c conection->isInitialized() is false.
    void initialize(const ReplierProcessOptions &options,
                    std::unique_ptr<Replier> &&replierConnection);
    /// @result True indicates the publisher process is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The name of the process.
    [[nodiscard]] std::string getName() const noexcept override;
    /// @}

    /// @name Step 3: Stop the Process
    /// @{

    /// @brief Stops the replier process.  This will send a 
    ///        shutdown message to the command module replier thread.
    void stop() override;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~ReplierProcess() override;
    /// @}
private:
    class ReplierProcessImpl;
    std::unique_ptr<ReplierProcessImpl> pImpl;
};
/// @brief Creates a replier process from options in the initialization file.
/// @param[in] requestor  The connection information requestor.
/// @param[in] iniFile    The initialization file.
/// @param[in] section    The section of the initialization file with the
///                       heartbeat options.
/// @param[in] context    The ZeroMQ context.
/// @param[in] logger     The application's logger.
/// @result On successful exit, this is a module registry process that is
///         connected and ready to reply to remote module interaction 
///         commands.
/// @ingroup UMPS_ProxyServices_Command
std::unique_ptr<UMPS::ProxyServices::Command::ReplierProcess>
    createReplierProcess(const UMPS::Services::ConnectionInformation::Requestor &requestor,
                         const std::string &iniFile,
                         const std::string &section = "ModuleRegistry",
                         std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                         std::shared_ptr<UMPS::Logging::ILog> logger = nullptr);
}
#endif
