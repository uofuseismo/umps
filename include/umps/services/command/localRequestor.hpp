#ifndef UMPS_SERVICES_COMMAND_LOCALREQUESTOR_HPP
#define UMPS_SERVICES_COMMAND_LOCALREQUESTOR_HPP
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
   class LocalRequestorOptions;
   class AvailableCommandsResponse;
   class CommandRequest;
   class CommandResponse;
  }
}
namespace UMPS::Services::Command
{
/// @class LocalRequestor "localRequestor.hpp" "umps/services/command/localRequestor.hpp"
/// @brief This class allows the user to interact with programs running locally
///        via inter-process communication.  Note, the program to which to
///        communicate must be running and have opened an IPC reply service
///        from which to receive requests.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class LocalRequestor
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    LocalRequestor();
    /// @brief Constructor with a given context.
    explicit LocalRequestor(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given logger.
    explicit LocalRequestor(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context and logger.
    LocalRequestor(std::shared_ptr<UMPS::Messaging::Context> &context,
                   std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] requestor  The requestor from which to create this class.
    ///                           On exit, requestor's behavior is undefined.
    LocalRequestor(LocalRequestor &&requestor) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Move assignment oeprator.
    /// @param[in,out] requestor  The requestor whose memory will be moved
    ///                           to this.  On exit, requestor's behavior
    ///                           is undefined.
    /// @result The memory from the requestor moved to this.
    LocalRequestor& operator=(LocalRequestor &&requestor) noexcept;
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes and connects the requestor to the service. 
    /// @param[in] options  The requestor options.  At a minimum this must
    ///                     have the module name.
    /// @throws std::invalid_argument if the module name is not set.
    /// @throws std::runtime_error if the application cannot connect.
    void initialize(const LocalRequestorOptions &options);
    /// @result True indicates the requestor is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Usage
    /// @{

    /// @brief Gets the commands for interacting with this program.
    /// @result A message summarizing the options for interacting with
    ///         this program.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<AvailableCommandsResponse> getCommands() const;
    /// @brief Issues a command to the service. 
    /// @param[in] request  The request to issue to the service.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::unique_ptr<CommandResponse> issueCommand(const CommandRequest &request);
    /// @}
     
    /// @name Destructors
    /// @{

    /// @brief Disconnects from the service.
    void disconnect();
    /// @brief Destructor.
    ~LocalRequestor();
    /// @}

    LocalRequestor(const LocalRequestor &) = delete;
    LocalRequestor& operator=(const LocalRequestor &) = delete;
private:
    class LocalRequestorImpl;
    std::unique_ptr<LocalRequestorImpl> pImpl;    
};
}
#endif
