#ifndef UMPS_SERVICES_COMMAND_REMOTEREQUESTOR_HPP
#define UMPS_SERVICES_COMMAND_REMOTEREQUESTOR_HPP
#include <memory>
namespace UMPS
{
 namespace Messaging
 {
  class Context;
  class IMessage;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Services::Command
  {
   class RemoteRequestorOptions;
   class AvailableCommandsResponse;
   class AvailableModulesResponse;
   class CommandRequest;
   class CommandResponse;
   class TerminateRequest;
   class TerminateResponse;
  }
}
namespace UMPS::Services::Command
{
/// @class RemoteRequestor "remoteRequestor.hpp" "umps/services/command/localRequestor.hpp"
/// @brief This class allows the user to interact with programs running remotely
///        i.e., - on a different machine.  Nominally, you will only want this
///        to be used by privileged users that you trust.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RemoteRequestor
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RemoteRequestor();
    /// @brief Constructor with a given context.
    explicit RemoteRequestor(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given logger.
    explicit RemoteRequestor(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context and logger.
    RemoteRequestor(std::shared_ptr<UMPS::Messaging::Context> &context,
                   std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] requestor  The requestor from which to create this class.
    ///                           On exit, requestor's behavior is undefined.
    RemoteRequestor(RemoteRequestor &&requestor) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Move assignment oeprator.
    /// @param[in,out] requestor  The requestor whose memory will be moved
    ///                           to this.  On exit, requestor's behavior
    ///                           is undefined.
    /// @result The memory from the requestor moved to this.
    RemoteRequestor& operator=(RemoteRequestor &&requestor) noexcept;
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes and connects the requestor to the service. 
    /// @param[in] options  The requestor options.  At a minimum this must
    ///                     have the module name.
    /// @throws std::invalid_argument if the module name is not set.
    /// @throws std::runtime_error if the application cannot connect.
    void initialize(const RemoteRequestorOptions &options);
    /// @result True indicates the requestor is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Usage
    /// @{

    /// @brief Gets the available registered modules with which to communicate.
    /// @result A message with the available modules.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<AvailableModulesResponse> getAvailableModules() const;
    /// @brief Gets the commands for interacting with this program.
    /// @result A message summarizing the options for interacting with
    ///         this program.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<AvailableCommandsResponse> getCommands() const;
    /// @brief Issues a command to the program.
    /// @param[in] request  The request to issue to the program.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<CommandResponse> issueCommand(const CommandRequest &request);
    /// @brief Issues a remote-termination command to the program.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<TerminateResponse> issueTerminateCommand() const;
    /// @}
     
    /// @name Destructors
    /// @{

    /// @brief Disconnects from the service.
    void disconnect();
    /// @brief Destructor.
    ~RemoteRequestor();
    /// @}

    RemoteRequestor(const RemoteRequestor &) = delete;
    RemoteRequestor& operator=(const RemoteRequestor &) = delete;
private:
    class RemoteRequestorImpl;
    std::unique_ptr<RemoteRequestorImpl> pImpl;    
};
}
#endif
