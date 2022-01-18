#ifndef UMPS_BROADCASTS_HEARTBEAT_BROADCAST_HPP
#define UMPS_BROADCASTS_HEARTBEAT_BROADCAST_HPP
#include <memory>
#include "umps/broadcasts/broadcast.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Services::ConnectionInformation
 {
  class Details;
 }
 namespace Broadcasts::Heartbeat
 {
  class Parameters;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::Broadcasts::Heartbeat
{
/// @class Broadcast "broadcast.hpp" "umps/broadcasts/heartbeat/broadcast.hpp"
/// @brief This defines the XPUB/XSUB proxy to broadcast heartbeats.
///        This is an intermediate message layer to which producers
///        can publish messages to this XSUB socket and from which
///        subscribers can receive content from this XPUB socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Broadcast : public IBroadcast
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructs the XPUB/XSUB proxy.
    Broadcast();
    /// @brief Constructs the proxy with a given logger.
    explicit Broadcast(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs the given authenticator. 
    explicit Broadcast(std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructs the proxy with a given logger and authenticator.
    Broadcast(std::shared_ptr<UMPS::Logging::ILog> &logger,
              std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] broadcast  The broadcast whose memory will be moved
    ///                           to this.  On exit, broadcast's will be
    ///                           undefined.
    /// @result The memory from broadcast moved to this.
    //Broadcast& operator=(Broadcast &&broadcast) noexcept;
    /// @}

    /// @brief Initializes the proxy.
    void initialize(const Parameters &parameters);

    /// @name Implementations required for an IBroadcast
    /// @{
    /// @result True indicates that the broadcast is initialized.
    [[nodiscard]] bool isInitialized() const noexcept override final;
    /// @result The name of the broadcast.
    [[nodiscard]] std::string getName() const override final;
    /// @result The connection details.
    [[nodiscard]] Services::ConnectionInformation::Details
        getConnectionDetails() const override final;
    /// @}

    /// @name Proxy Management
    /// @{

    /// @brief Starts the proxy and authenticator service.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start() override final;
    /// @result True indicates the proxy and authenticator are running.
    bool isRunning() const noexcept;
    /// @brief Stops the proxy and authenticator.
    void stop() override final;
    /// @}

    /// @result An uninitialized instance of this class.
    [[nodiscard]] std::unique_ptr<IBroadcast> createInstance() const noexcept override final;

    /// @name Destructors
    /// @{
    virtual ~Broadcast();
    /// @}

    Broadcast(const Broadcast &broadcast);
    Broadcast(Broadcast &&broadcast) noexcept = delete;
    Broadcast& operator=(const Broadcast &broadcast) = delete;
    Broadcast& operator=(Broadcast &&broadcast) noexcept = delete;
private:
    class BroadcastImpl;
    std::unique_ptr<BroadcastImpl> pImpl;
};
};
#endif
