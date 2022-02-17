#ifndef UMPS_PROXYSERVICES_PACKETCACHE_INTERPOLATE_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_INTERPOLATE_HPP
#include <vector>
#include "umps/proxyServices/packetCache/enums.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  template<class T> class DataPacket;
 }
}
namespace UMPS::ProxyServices::PacketCache
{
/// @brief Interpolates a collection of data packets.  This will interpolate 
///        from the earliest time in the collection of packets up to the
///        the last time.
/// @param[in] packets   The packets to interpolate.
/// @param[in] targetSamplingRate  The sampling rate of the output signal in Hz.
/// @result A container with the interpolated signal.
/// @note The interpolation is actually performed in double precision.  For
///       that reason there can be a significant loss of precision if
///       using the integer template variant.
template<typename T>
[[nodiscard]]
MessageFormats::DataPacket<T>
interpolate(const std::vector<MessageFormats::DataPacket<T>> &packets,
            double targetSamplingRate = 100);
}
#endif
