#ifndef UMPS_PROXY_SERVICES_COMMAND_REPLIER_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REPLIER_HPP
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
  class Request;
 }
 namespace ProxyServices::Command
 {
  class ReplierOptions;
 }
}
namespace UMPS::ProxyServices::Command
{
/// @class Replier "replier.hpp" "umps/proxyServices/command/replier.hpp"
/// @brief A replier mechanism for the server in the remote command service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_uRemoteCommand
class Replier
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Replier();
    /// @brief Constructs a reply socket with a given context.
    explicit Replier(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructs a reply socket with the given logger.
    explicit Replier(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a reply socket with a given logger and context.
    Replier(std::shared_ptr<UMPS::Messaging::Context> &context,
                  std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the reply.
    /// @param[in] options  The reply options.
    /// @throws std::invalid_argument if the endpoint is not set.
    void initialize(const ReplierOptions &options);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for binding to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This is a router-router pattern which means the frontend and
    ///       backend are both request sockets.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Request getSocketDetails() const;
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
    ~Replier();
    /// @}

    Replier(const Replier &) = delete;
    Replier(Replier &&) noexcept = delete;
    Replier& operator=(const Replier &) = delete;
    Replier& operator=(Replier &&) noexcept = delete;
private:
    class ReplierImpl;
    std::unique_ptr<ReplierImpl> pImpl;
};
}
#endif
