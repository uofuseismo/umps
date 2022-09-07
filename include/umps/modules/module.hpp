#ifndef UMPS_MODULES_MODULE_HPP
#define UMPS_MODULES_MODULE_HPP
#include <memory>
#include <chrono>
#include "umps/logging/level.hpp"
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
 namespace Services
 {
  namespace ConnectionInformation
  {
   class Requestor;
  }
 }
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
/// @class Module "module.hpp" "umps/modules/module.hpp"
/// @brief This is a generic base class module.  It will implement some, but not
///        all, generic module activities.  Of course, any and all routines
///        can be overridden as necessary.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Modules_ModuleBaseClass
class IModule
{
public:
    /// @brief Constructor.
    IModule();

    /// @name Initialization
    /// @{

    /// @brief Parses an initialization file and extracts the uOperator 
    ///        information from the uOperator section, standard general
    ///        module information from the module General section,
    ///        and the heartbeat broadcast, and command information.
    ///        Typical general items are the module name, verbosity, and
    ///        log directory.  Typical heartbeat properties are the
    ///        heartbeat interval and, if necessary, the heartbeat broadcast
    ///        name.
    /// @param[in] initializationFile  The name of the initialization file.
    virtual void parseInitializationFile(const std::string &initializationFile);

    /// @brief Sets the module's name.
    virtual void setName(const std::string &name); 
    /// @result The module's name.
    virtual std::string getName() const noexcept;

    /// @brief Sets the operator address. 
    /// @param[in] address  The uOperator address - e.g., tcp://127.0.0.1:8080
    virtual void setOperatorAddress(const std::string &address);
    /// @result The operator address.
    /// @throws std::runtime_error if the operator address was not set.
    [[nodiscard]] virtual std::string getOperatorAddress() const;
    /// @result True indicates the operator address was set.
    [[nodiscard]] virtual bool haveOperatorAddress() const noexcept;
 
    /// @brief Sets the ZeroMQ authentication options.
    /// @param[in] options  The ZAP options.
    virtual void setZAPOptions(const UMPS::Authentication::ZAPOptions &options) noexcept;
    /// @result The ZAP options.
    [[nodiscard]] virtual UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;

    /// @brief Sets the name of the heartbeat broadcast.
    /// @param[in] name  The name of the heartbeat broadcast.
    /// @throws std::invalid_argument if the name is empty.
    virtual void setHeartbeatBroadcastName(const std::string &name);
    /// @result The name of the heartbeat broadcast.  By default the broadcast
    ///         is named Heartbeat and this will always be made available
    ///         by the 
    [[nodiscard]] virtual std::string getHeartbeatBroadcastName() const noexcept;

    /// @brief Sets the interval between status messages in the
    ///        heartbeat broadcast.
    /// @param[in] interval  The interval in seconds.  This must be positive.
    /// @throws std::invalid_argument if the interval is not positive.
    virtual void setHeartbeatInterval(const std::chrono::seconds &interval);
    /// @result The interval between status messages in the heartbeat broadcast.
    [[nodiscard]] std::chrono::seconds getHeartbeatInterval() const noexcept;

    /// @brief Sets the module verbosity.
    /// @param[in] verbosity  The logging leve.
    virtual void setVerbosity(UMPS::Logging::Level verbosity) noexcept;
    /// @result The module's verbosity.
    [[nodiscard]] virtual UMPS::Logging::Level getVerbosity() const noexcept;

    /// @brief Sets the log file directory.
    /// @param[in] directory  The file directory.
    /// @throws std::runtime_error if the log file directory cannot be created.
    virtual void setLogFileDirectory(const std::string &directory);
    /// @result The log file directory.   By default this will be /var/log/umps.
    [[nodiscard]] virtual std::string getLogFileDirectory() const noexcept;

    /// @brief Sets the application's logger.
    /// @param[in] logger  A pointer to the application's logger.
    virtual void setLogger(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @result A pointer to the application's logger.
    /// @note By default this is a stdout logger.
    [[nodiscard]] virtual std::shared_ptr<UMPS::Logging::ILog> getLogger() const noexcept;
    /// @}


    /// @name Connect
    /// @{

    /// @brief This will establish a connection to the 
    /// @throws std::runtime_error if \c haveSetOperatorAddress() is false.
    /// @throws std::runtime_error if there are issues connecting to uOperator.
    virtual void connect();
    /// @result True indicates that the uOperator connection is established. 
    [[nodiscard]] virtual bool isConnected() const noexcept;

    /// @result A pointer to the connection information requestor.
    ///         From this, uOperator can be queried for other connection
    ///         information.
    /// @throws std::runtime_error if \c isConnected() is false.
    [[nodiscard]] std::shared_ptr<UMPS::Services::ConnectionInformation::Requestor> getConnectionInformationRequestor() const;
    
    /// @}

    /// @brief Starts the threads handling the heartbeating and commands.
    virtual void start();
    /// @brief Stops the threads handling the heartbeating and commands. 
    virtual void stop();
    /// @result True indicates the modules should continue to run.
    [[nodiscard]] bool keepRunning() const noexcept;
    //void sendStatus( );
    /// @brief Disconnects from the status and 
    void disconnect();
    

    /// @brief Destructor.
    virtual ~IModule();

    IModule& operator=(const IModule &) = delete;
    IModule& operator=(IModule &&) noexcept = delete;
    IModule(const IModule &) = delete;
    IModule(IModule &&) noexcept = delete;
private:
    class IModuleImpl;
    std::unique_ptr<IModuleImpl> pImpl;
};
}
#endif
