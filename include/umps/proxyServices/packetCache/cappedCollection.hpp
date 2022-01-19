#ifndef UMPS_PROXYSERVICES_PACKETCACHE_CAPPEDCOLLECTION_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_CAPPEDCOLLECTION_HPP
#include <memory>
#include <unordered_set>
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
namespace UMPS::ProxyServices::PacketCache
{
/// @class CappedCollection "cappedCollection.hpp" "umps/proxyServices/packetCache/cappedCollection.hpp"
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

    /// @name Adding Packets 
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
    /// @param[in] name  The name of the station in 
    ///                  NETWORK.STATION.CHANNEL.LOCATION_CODE format.
    /// @result True indicates that the sensor exists in the collection.
    [[nodiscard]] bool haveSensor(const std::string &name) const noexcept;
    /// @result All the sensors currently in the capped collection.
    /// @note The names of each sensor are formatted as:
    ///       NETWORK.STATION.CHANNEL.LOCATION_CODE.
    [[nodiscard]] std::unordered_set<std::string> getSensorNames() const noexcept;
    /// @}

    /// @name Querying Packets 
    /// @{

    /// @brief Returns the start time of the earliest packet in the buffer.
    /// @throws std::runtime_error \c isInitialized() is false or the 
    ///         \c getNumberOfPackets() is zero. 
    [[nodiscard]] std::chrono::microseconds getEarliestStartTime(const std::string &name) const;
    /// @brief Gets all packets beginning at time t0.
    /// @param[in] name  The name of the channel.
    /// @param[in] t0    The UTC start time of the query in seconds since
    ///                  the epoch.
    /// @result All packets from time t0 to the most recent packet.
    /// @note If data younger than t0 has expired then the oldest sample in 
    ///       the buffer will be the first element of the result.
    /// @throws std::invalid_argument if \c haveSensor(name) is false.
    [[nodiscard]] std::vector<UMPS::MessageFormats::DataPacket<T>> getPackets(const std::string &name, double t0) const;
    /// @param[in] name  The name of the channel.
    /// @param[in] t0    The UTC start time of the query in microseconds since
    ///                  the epoch.
    /// @result All packets from time t0 to the most recent packet for the
    ///         given station.
    /// @throws std::invalid_argument if \c haveSensor(name) is false.
    [[nodiscard]] std::vector<UMPS::MessageFormats::DataPacket<T>>
        getPackets(const std::string &name, const std::chrono::microseconds &t0) const;
    /// @brief Gets all packets between time t0 and t1.
    /// @param[in] name  The name of the channel.
    /// @param[in] t0    The UTC start time of the query in seconds since
    ///                  the epoch.
    /// @param[in] t1    The UTC end time of the query in seconds since
    ///                  the epoch.
    /// @result All packets from t0 to t1.
    /// @throws std::invalid_argument if t0 >= t1.
    [[nodiscard]] std::vector<UMPS::MessageFormats::DataPacket<T>>
        getPackets(const std::string &name, double t0, double t1) const;
    /// @param[in] name  The name of the channel.
    /// @param[in] t0    The UTC start time of the query in microseconds
    ///                  since the epoch.
    /// @param[in] t1    The UTC end time of the query in micrsoseconds
    ///                  since the epoch.
    /// @result All packets from t0 to t1.
    /// @throws std::invalid_argument if t0 >= t1.
    [[nodiscard]] std::vector<UMPS::MessageFormats::DataPacket<T>>
        getPackets(const std::string &name,
                   const std::chrono::microseconds &t0,
                   const std::chrono::microseconds &t1) const;
    /// @result All the datapackets in the buffer.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::vector<UMPS::MessageFormats::DataPacket<T>>
        getPackets(const std::string &name) const;
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

    CappedCollection(const CappedCollection &) = delete;
    CappedCollection& operator=(const CappedCollection &) = delete;
    CappedCollection(CappedCollection &&) = delete;
    CappedCollection& operator=(CappedCollection &&) noexcept = delete;
private:
    class CappedCollectionImpl;
    std::unique_ptr<CappedCollectionImpl> pImpl;
};
}
#endif
