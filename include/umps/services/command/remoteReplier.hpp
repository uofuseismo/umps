#ifndef UMPS_SERVICES_COMMAND_REMOTEREPLIER_HPP
#define UMPS_SERVICES_COMMAND_REMOTEREPLIER_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Reply;
 }
 namespace Services::Command
 {
  class RemoteReplierOptions;
 }
}
namespace UMPS::Services::Command
{
/// @class Replier "replier.hpp" "umps/proxyServices/incrementer/replier.hpp"
/// @brief A replier mechanism for the server in the incrementer.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RemoteReplier
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RemoteReplier();
    /// @brief Constructs a reply socket with a given context.
    explicit RemoteReplier(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructs a reply socket with the given logger.
    explicit RemoteReplier(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a reply socket with a given logger and context.
    RemoteReplier(std::shared_ptr<UMPS::Messaging::Context> &context,
                  std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the reply.
    /// @param[in] options  The reply options.
    /// @throws std::invalid_argument if the endpoint is not set.
    void initialize(const RemoteReplierOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Reply getSocketDetails() const;
    /// @}

    /// @name Step 2: Start the Replier Service
    /// @{

    /// @brief Starts a thread to respond to requests from the dealer.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start();
    /// @result True indicates that the reply service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @}

    /// @name Step 3: Stop the Replier Service
    /// @{

    /// @brief This will stop the reply service.
    void stop();
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~RemoteReplier();
    /// @}

    RemoteReplier(const RemoteReplier &) = delete;
    RemoteReplier(RemoteReplier &&) noexcept = delete;
    RemoteReplier& operator=(const RemoteReplier &) = delete;
    RemoteReplier& operator=(RemoteReplier &&) noexcept = delete;
private:
    class RemoteReplierImpl;
    std::unique_ptr<RemoteReplierImpl> pImpl;
};
}
#endif
