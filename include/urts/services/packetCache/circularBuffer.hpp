#ifndef URTS_SERVICES_PACKETCACHE_CIRCULARBUFFER_HPP
#define URTS_SERVICES_PACKETCACHE_CIRCULARBUFFER_HPP
#include <memory>
#include <vector>
namespace URTS
{
 namespace MessageFormats
 {
  template<class T> class DataPacket;
 }
}
namespace URTS::Services::PacketCache
{
/// @class CircularBuffer "circularBuffer.hpp" "urts/services/packetCache/circularBuffer.hpp"
/// @brief This is a thread-safe circular buffer for storing data packets for
///        a given station's channel.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T>
class CircularBuffer
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    CircularBuffer();
    /// @brief Copy constructor.
    /// @param[in] circularBuffer  The circularBuffer class from which to
    ///                            initialize this class. 
    CircularBuffer(const CircularBuffer &circularBuffer);
    /// @brief Move constructor.
    /// @param[in,out] circularBuffer  The circular buffer class from which to
    ///                                initialize this class.  On exit,
    ///                                circularBuffer's behavior is undefined.
    CircularBuffer(CircularBuffer &&circularBuffer) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] circularBuffer  The circularBuffer class to copy to this.
    /// @result A deep copy of circularBuffer.
    CircularBuffer& operator=(const CircularBuffer &circularBuffer);
    /// @brief Move assignment operator.
    /// @param[in,out] circularBuffer  The circularBuffer whose memory will
    ///                                be moved to this.  On exit,
    ///                                circularBuffers's behavior is undefined.
    /// @result The memory from circularBuffer moved to this.
    CircularBuffer& operator=(CircularBuffer &&circularBuffer) noexcept;
    /// @}
     
    /// @brief Initialization
    /// @{
    /// @param[in] network       The network code - e.g., UU.
    /// @param[in] station       The station name - e.g., FSU.  
    /// @param[in] channel       The channel name - e.g., EHZ.
    /// @param[in] locationCode  The location code - e.g., 01.
    /// @param[in] maxPackets    The maximum number of packets in the circular
    ///                          buffer.
    /// @throws std::invalid_argument if maxPackets is not positive or either
    ///         the network, station, channel, or locationCode is emtpy
    ///         or blank.
    void initialize(const std::string &network,
                    const std::string &station,
                    const std::string &channel,
                    const std::string &locationCode,
                    int maxPackets);
    /// @result True indicates that the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The network code.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::string getNetwork() const;
    /// @result The station code.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::string getStation() const;
    /// @result The channel code.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::string getChannel() const;
    /// @result The location code.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::string getLocationCode() const;
    /// @result The maximum number of packets the circular buffer can hold.
    [[nodiscard]] int getMaximumNumberOfPackets() const;
    /// @result The number of packets the cicrular buffer is currently holding.
    [[nodiscard]] int getNumberOfPackets() const noexcept;
    /// @}

    /// @name Adding Packets
    /// @{
    /// @brief Attempts to add the given packets to the buffer.
    /// @param[in] packets  The packets to add to this buffer.
    /// @sa \c addPacket() for rules defining a valid data packet.
    //void addPackets(const std::vector<URTS::MessageFormats::DataPacket<T>> &packets);
    /// @brief Attempts to add the given packet to the buffer.
    /// @param[in] packet   The packet to add to this buffer.
    /// @note The network, station, channel, and location code must match
    ///       the network, station, channel, and location code specified 
    ///       during initialization.  Additionally, the data packet must have
    ///       a positive sampling rate and data must actually exist on the
    ///       packet.
    void addPacket(const URTS::MessageFormats::DataPacket<T> &packet);
    /// @brief Attempts to add the given packet to the buffer.
    /// @param[in,out] packet   The packet to add to the buffer.  On exit,
    ///                         packet's behavior is undefined.
    /// @note The network, station, channel, and location code must match
    ///       the network, station, channel, and location code specified 
    ///       during initialization.  Additionally, the data packet must have
    ///       a positive sampling rate and data must actually exist on the
    ///       packet.
    void addPacket(URTS::MessageFormats::DataPacket<T> &&packet);
    /// @}

    /// @name Getting Packets
    /// @{
    /// @brief Returns the start time of the earliest packet in the buffer.
    /// @throws std::runtime_error \c isInitialized() is false or the 
    ///         \c getNumberOfPackets() is zero. 
    [[nodiscard]] int64_t getEarliestStartTime() const;
    /// @brief Gets all packets beginning at time t0.
    /// @param[in] t0  The UTC start time of the query in seconds since
    ///                the epoch.
    /// @result All packets from time t0 to the most recent packet.
    /// @note If data older than t0 has expired then the oldest sample in 
    ///       the buffer will be the first element of the result.
    [[nodiscard]] std::vector<URTS::MessageFormats::DataPacket<T>> getPackets(double t0) const;
    /// @param[in] t0  The UTC start time of the query in seconds since
    ///                the epoch.
    /// @param[in] t1  The UTC end time of the query in seconds since
    ///                the epoch.
    /// @throws std::invalid_argument if t0 >= t1.
    [[nodiscard]] std::vector<URTS::MessageFormats::DataPacket<T>> getPackets(double t0, double t1) const;
    /// @result All the datapackets in the buffer.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::vector<URTS::MessageFormats::DataPacket<T>> getPackets() const;
    /// @}

    /// @name Cleaning
    /// @{
    /// @brief Removes all packets that began before the given time. 
    //void removeExpiredPackets(double t0);
    /// @}

    /// @name Destructors
    /// @{ 
    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~CircularBuffer();
    /// @}
private:
    class CircularBufferImpl;
    std::unique_ptr<CircularBufferImpl> pImpl; 
};
}
#endif