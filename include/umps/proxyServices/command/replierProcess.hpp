#ifndef UMPS_PROXY_SERVICES_COMMAND_REPLIER_PROCESS_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REPLIER_PROCESS_HPP
#include <memory>
#include <chrono>
#include <functional>
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
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace ProxyServices::Command
 {
  class ModuleDetails;
  class ReplierOptions;
  class Replier;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class ReplierPRocess "replierPRocess.hpp" "umps/proxyServices/command/replierProcess.hpp"
/// @brief This process will respond to remote module interaction messages.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_uRemoteCommand
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
    ReplierProcess(std::shared_ptr<UMPS::Messaging::Context> &context,
                   std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the replier process.
    /// @param[in] options  The replier process options.
    /// @throws std::invalid_argument if the module details or connection
    ///         address is not set.
    /// @throws std::runtime_error if the replier process cannot be created.
    void initialize(const ReplierOptions &options);
    /// @result True indicates the publisher process is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The name of the process.
    [[nodiscard]] std::string getName() const noexcept override;
    /// @}

    /// @name Step 2: Run the Replier Process
    /// @{

    /// @brief Starts the replier process.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start() override;
    /// @result True indicates the process is running.
    [[nodiscard]] bool isRunning() const noexcept override;
    /// @}

    /// @name Step 3: Stop the Replier Process
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
/// @param[in] iniFile        The initialization file.
/// @param[in] moduleDetails  The module details.
/// @param[in] section        The section of the initialization file with the
///                           module registry options.
/// @param[in] context        The ZeroMQ context.
/// @param[in] logger         The application's logger.
/// @result On successful exit, this is a module registry process that is
///         connected and ready to reply to remote module interaction 
///         commands.
/// @throws std::invalid_argument if the module name in moduleDetails is not
///         set, the initialization file does not exist.
/// @thrwos std::runtime_error if the replier process cannot be created.
/// @ingroup UMPS_ProxyServices_Command
std::unique_ptr<ReplierProcess>
    createReplierProcess(const UMPS::Services::ConnectionInformation::Requestor &requestor,
                         const ModuleDetails &moduleDetails,
                         const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                 (const std::string &messageType, const void *data, size_t length)> &callback,
                         const std::string &iniFile,
                         const std::string &section = "ModuleRegistry",
                         std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                         std::shared_ptr<UMPS::Logging::ILog> logger = nullptr);
}
#endif
