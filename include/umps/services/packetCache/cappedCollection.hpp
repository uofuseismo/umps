#ifndef UMPS_SERVICES_PACKETCACHE_CAPPEDCOLLECTION_HPP
#define UMPS_SERVICES_PACKETCACHE_CAPPEDCOLLECTION_HPP
#include <memory>
#include <vector>
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace MessageFormats
 {
  template<class T> class DataPacket;
 }
}
namespace UMPS::Services::PacketCache
{
/// @class CappedCollection "cappedCollection.hpp" "umps/services/packetCache/cappedCollection.hpp"
/// @brief This is a thread-safe fixed-size collection of trace-packets for a
///        unique sensor (network, station, channel, location code).
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T>
class CappedCollection
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    CappedCollection();
    /// @brief Constructor.
    /// @param[in] logger  The logger for this class to use.
    explicit CappedCollection(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Initialization
    /// @{
    /// @brief Initializes the capped collection.
    /// @param[in] maxPackets  The maximum number of packets retained in
    ///                        this collection.
    void initialize(int maxPackets);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @}

    /// @name Adding Data
    /// @{
    /// @brief Adds a packet to the collection.
    /// @param[in] packet  The datapacket to add to the collection.
    /// @throws std::invalid_argument if the network, station, channel,
    ///         location code, sampling rate, or data is not set.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note If the underlying buffer is full and the data is expired this
    ///       packet will not be added.
    void addPacket(const UMPS::MessageFormats::DataPacket<T> &packet);
    /// @brief Adds a packet to the collection.  This uses move semantics and
    ///        is slightly faster.
    /// @param[in,out] packet  The datapacket to add to the collection.
    ///                        On exit, packet's behavior is undefined.
    /// @throws std::invalid_argument if the network, station, channel,
    ///         location code, sampling rate, or data is not set.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note If the underlying buffer is full and the data is expired this
    ///       packet will not be added.
    void addPacket(UMPS::MessageFormats::DataPacket<T> &&packet);
    /// @result True indicates that the sensor exists in the collection.
    [[nodiscard]] bool haveSensor(const std::string &network,
                                  const std::string &station,
                                  const std::string &channel,
                                  const std::string &locationCode) const noexcept;
    /// @result All the sensors currently in the capped collection.
    /// @note The sensors names are formatted as:
    ///       NETWORK.STATION.CHANNEL.LOCATION_CODE.
    [[nodiscard]] std::vector<std::string> getSensorNames() const noexcept;
    /// @}

    /// @name Querying Data
    /// @{

    /// @}

    /// @result The total number of packets in all of the circular buffers.
    [[nodiscard]] int getTotalNumberOfPackets() const noexcept;

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~CappedCollection();
    /// @}
private:
    class CappedCollectionImpl;
    std::unique_ptr<CappedCollectionImpl> pImpl;
};
}
#endif
