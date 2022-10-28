#ifndef UMPS_SERVICES_COMMAND_REQUESTOR_HPP
#define UMPS_SERVICES_COMMAND_REQUESTOR_HPP
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
   class RequestorOptions;
   class AvailableCommandsResponse;
   class CommandRequest;
   class CommandResponse;
   class TerminateRequest;
   class TerminateResponse;
  }
}
namespace UMPS::Services::Command
{
/// @class Requestor "requestor.hpp" "umps/services/command/requestor.hpp"
/// @brief This class allows the user to interact with programs running locally
///        via inter-process communication.  Note, the program to which to
///        communicate must be running and have opened an IPC reply service
///        from which to receive requests.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Requestor
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Requestor();
    /// @brief Constructor with a given context.
    explicit Requestor(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given logger.
    explicit Requestor(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context and logger.
    Requestor(std::shared_ptr<UMPS::Messaging::Context> &context,
                   std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] requestor  The requestor from which to create this class.
    ///                           On exit, requestor's behavior is undefined.
    Requestor(Requestor &&requestor) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Move assignment oeprator.
    /// @param[in,out] requestor  The requestor whose memory will be moved
    ///                           to this.  On exit, requestor's behavior
    ///                           is undefined.
    /// @result The memory from the requestor moved to this.
    Requestor& operator=(Requestor &&requestor) noexcept;
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes and connects the requestor to the service. 
    /// @param[in] options  The requestor options.  At a minimum this must
    ///                     have the module name.
    /// @throws std::invalid_argument if the module name is not set.
    /// @throws std::runtime_error if the application cannot connect.
    void initialize(const RequestorOptions &options);
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
    ~Requestor();
    /// @}

    Requestor(const Requestor &) = delete;
    Requestor& operator=(const Requestor &) = delete;
private:
    class RequestorImpl;
    std::unique_ptr<RequestorImpl> pImpl;    
};
}
#endif
