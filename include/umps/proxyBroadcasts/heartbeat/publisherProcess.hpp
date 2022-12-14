#ifndef UMPS_PROXY_BROADCASTS_HEARTBEAT_PUBLISHER_PROCESS_HPP
#define UMPS_PROXY_BROADCASTS_HEARTBEAT_PUBLISHER_PROCESS_HPP
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
 namespace ProxyBroadcasts::Heartbeat
 {
  class PublisherProcessOptions;
  class Publisher;
  class Status;
 }
}
namespace UMPS::ProxyBroadcasts::Heartbeat
{
/// @class PublisherProcess "publisherProcess.hpp" "umps/proxyBroadcasts/heartbeat/publisherProcess.hpp"
/// @brief This process will send status messages to the heartbeat broadcast
///        at regular intervals. 
/// @copyright Ben Baker (University of Utah) distributed under the MIT lciense.
/// @ingroup UMPS_ProxyBroadcasts_Heartbeat
class PublisherProcess : public UMPS::Modules::IProcess
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    PublisherProcess();
    /// @brief Constructor with a given logger.
    explicit PublisherProcess(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given context.
    //explicit PublisherProcess(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a given logger and context.
    //PublisherProcess(std::shared_ptr<UMPS::Messaging::Context> &context,
    //                 std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the process options.
    /// @param[in] options              The publisher process options.
    /// @param[in] publisherConnection  The connection to the publisher.
    /// @throws std::invalid_argument if \c conection->isInitialized() is false.
    void initialize(const PublisherProcessOptions &options,
                    std::unique_ptr<Publisher> &&publisherConnection); 
    /// @result True indicates the publisher process is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The name of the process.
    [[nodiscard]] std::string getName() const noexcept override;
    /// @}

    /// @name Step 2: Start the Process
    /// @{
    
    /// @brief Starts the heartbeat publisher process.
    void start() override;
    /// @result True indicates the process is running.
    [[nodiscard]] bool isRunning() const noexcept override;
    /// @brief Sets the status message that will be broadcast at regular
    ///        intervals.  By default, the process will send messages to
    ///        indicate that it is running but you can override that.
    /// @throws std::runtime_error if \c isRunning() is false.
    void setStatus(const Status &status);
    /// @brief By default this process will send messages at regular intervals.
    ///        However, if you feel compelled to, this function will allow you
    ///        to send a status.
    /// @throws std::runtime_error if \c isRunning() is false.
    void sendStatus(const Status &status) const; 
    /// @}

    /// @name Step 3: Stop the Process
    /// @{

    /// @brief Stops the heartbeat publisher process.  This will send a 
    ///        shutdown message to the broadcast.
    void stop() override;
    /// @}

    /// @name Destructors
    /// @{
 
    /// @brief Destructor.
    ~PublisherProcess() override;
    /// @}

    PublisherProcess(const PublisherProcess &) = delete;
    PublisherProcess(PublisherProcess &&) noexcept = delete;
    PublisherProcess& operator=(const PublisherProcess &) = delete;
    PublisherProcess& operator=(PublisherProcess &&) noexcept = delete;
private:
    class PublisherProcessImpl;
    std::unique_ptr<PublisherProcessImpl> pImpl;
};
/// @brief Creates a heartbeat process from options in the initialization
///        file.
/// @param[in] requestor  The connection information requestor.
/// @param[in] iniFile    The initialization file.
/// @param[in] section    The section of the initialization file with the
///                       heartbeat options.
/// @param[in] context    The ZeroMQ context.
/// @param[in] logger     The application's logger.
/// @result On successful exit, this is a heartbeat process that is connected
///         and ready to publish to the heartbeat broadcast.  
std::unique_ptr<UMPS::ProxyBroadcasts::Heartbeat::PublisherProcess>
    createHeartbeatProcess(const UMPS::Services::ConnectionInformation::Requestor &requestor,
                           const std::string &iniFile,
                           const std::string &section = "Heartbeat",
                           std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                           std::shared_ptr<UMPS::Logging::ILog> logger = nullptr);
}
#endif
