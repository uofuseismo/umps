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

   
    /// @name Destructors
    /// @{

    /// @brief Disconnects from the service.
    void disconnect();
    /// @brief Destructor.
    ~LocalRequestor();
    /// @}

    LocalRequestor(const LocalRequestor &) = delete;
    LocalRequestor(LocalRequestor &&) noexcept = delete;
    LocalRequestor& operator=(const LocalRequestor &) = delete;
    LocalRequestor& operator=(LocalRequestor &&) noexcept = delete;
private:
    class LocalRequestorImpl;
    std::unique_ptr<LocalRequestorImpl> pImpl;    
};
}
#endif
