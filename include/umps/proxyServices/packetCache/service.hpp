#ifndef UMPS_PROXYSERVICES_PACKETCACHE_SERVICE_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_SERVICE_HPP
#include <memory>
//#include "umps/messageFormats/message.hpp"
#include "umps/proxyServices/packetCache/enums.hpp"
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace ProxyBroadcasts::DataPacket
 {
  template<class T> class SubscriberOptions;
 }
 namespace ProxyServices::PacketCache
 {
  class ReplierOptions;
 } 
}
namespace UMPS::ProxyServices::PacketCache
{
/// @name Service "service.hpp" "umps/proxyServices/packetCache/service.hpp"
/// @brief This is a container that comprises the backend service.  It will
///        read packets from a data feed, put those packets into a capped
///        collection, and, when requested, send those packets to clients.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
class Service
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Service();
    /// @brief Constructor with given logger.
    explicit Service(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes the class.
    void initialize(int maxPackets,
                    const ProxyBroadcasts::DataPacket::SubscriberOptions<T> &subscriberOptions,
                    const ReplierOptions &replyOptions);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Start/Stop Service
    /// @{

    /// @brief Starts the service.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start();
    /// @result True indicates the service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @result The total number of packets in the packet cache.
    [[nodiscard]] int getTotalNumberOfPackets() const noexcept;
    /// @brief Stops the service.
    void stop();
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Service();
    /// @}


    Service(const Service &) = delete;
    Service(Service &&) noexcept = delete;
    Service& operator=(const Service &) = delete;
    Service& operator=(Service &&) noexcept = delete;
private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl;
};
}
#endif
