#ifndef UMPS_BROADCASTS_DATAPACKET_BROADCAST_HPP
#define UMPS_BROADCASTS_DATAPACKET_BROADCAST_HPP
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
 namespace Broadcasts::DataPacket
 {
  class Parameters;
 }
}
namespace UMPS::Broadcasts::DataPacket
{
/// @class Broadcast "broadcast.hpp" "umps/broadcasts/dataPacket/broadcast.hpp"
/// @brief This defines the XPUB/XSUB proxy to broadcast data packets.
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
    /// @brief Move constructor.
    /// @param[in,out] broadcast  The broadcast from which to initialize 
    ///                           this class.  On exit, broadcasts's behavior
    ///                           is undefined.
    Broadcast(Broadcast &&broadcast) noexcept;
    /// @brief Constructs the XPUB/XSUB with a given logger.
    explicit Broadcast(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] broadcast  The broadcast whose memory will be moved
    ///                           to this.  On exit, broadcast's will be
    ///                           undefined.
    /// @result The memory from broadcast moved to this.
    Broadcast& operator=(Broadcast &&broadcast) noexcept;
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

    /// @name Starts the proxy.
    /// @{
    /// @brief Starts the proxy.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start() override final;
    /// @result True indicates the proxy is running.
    bool isRunning() const noexcept;
    /// @brief Stops the proxy.
    void stop() override final;
    /// @}

    /// @result An uninitialized instance of this class.
    [[nodiscard]] std::unique_ptr<IBroadcast> createInstance() const noexcept override final;

    /// @name Destructors
    /// @{
    virtual ~Broadcast();
    /// @}

    Broadcast(const Broadcast &broadcast);
    Broadcast& operator=(const Broadcast &broadcast) = delete;
private:
    class BroadcastImpl;
    std::unique_ptr<BroadcastImpl> pImpl;
};
}
#endif
