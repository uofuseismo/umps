#ifndef UMPS_PROXYBROADCASTS_HEARTBEAT_PUBLISHERPROCESS
#define UMPS_PROXYBROADCASTS_HEARTBEAT_PUBLISHERPROCESS
#include "umps/modules/process.hpp"
#include <memory>
#include <chrono>
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
 }
}
namespace UMPS::ProxyBroadcasts::Heartbeat
{
/// @brief This process will send status messages to the heartbeat broadcast
///        at regular intervals. 
class PublisherProcess : public UMPS::Modules::IProcess
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    explicit PublisherProcess(std::shared_ptr<UMPS::Logging::ILog> logger = nullptr);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the process options.
    /// @param[in] options     The publisher process options.
    /// @param[in] connection  The connection.
    /// @throws std::invalid_argument if \c conection->isInitialized() is false.
    void initialize(const PublisherProcessOptions &options,
                    std::unique_ptr<Publisher> &&publisherConnection); 
    /// @result True indicates the publisher process is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Step 2: Start the Process
    /// @{
    
    /// @brief Starts the heartbeat publisher process.
    void start() override;
    /// @result True indicates the process is running.
    [[nodiscard]] bool isRunning() const noexcept override;
    /// @brief Sends a status message.
    //[[nodiscard]] void 
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
    createHeartbeatProcess(std::shared_ptr<UMPS::Services::ConnectionInformation::Requestor> &requestor,
                           const std::string &iniFile,
                           const std::string &section = "Heartbeat",
                           std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                           std::shared_ptr<UMPS::Logging::ILog> logger = nullptr);
}
#endif
