#ifndef URTS_APPLICATIONS_PACKETCACHE_CAPPEDCOLLECTION_HPP
#define URTS_APPLICATIONS_PACKETCACHE_CAPPEDCOLLECTION_HPP
#include <memory>
#include <vector>
namespace URTS
{
// namespace Logging
// {
//  class ILogger;
// }
 namespace MessageFormats
 {
  template<class T> class DataPacket;
 }
}
namespace URTS::Applications::PacketCache
{
/// @class CappedCollection "cappedCollection.hpp" "urts/applications/packetCache/cappedCollection.hpp"
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
    /// @}

    /// @param[in] packet  The datapacket to add to the collection.
    /// @throws std::invalid_argument if the network, station, channel,
    ///         location code, sampling rate, or data is not set.
    /// @note If the underlying buffer is full and the data is expired this
    ///       packet will not be added.
    void addPacket(const URTS::MessageFormats::DataPacket<T> &packet);
    /// @result True indicates that the sensor exists in the collection.
    [[nodiscard]] bool haveSensor(const std::string &network,
                                  const std::string &station,
                                  const std::string &channel,
                                  const std::string &locationCode) const noexcept;
private:
    class CappedCollectionImpl;
    std::unique_ptr<CappedCollectionImpl> pImpl;
};
}
#endif
