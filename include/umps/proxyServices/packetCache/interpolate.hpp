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
/// @brief Interpolates a collection of data packets.
template<typename T> 
void interpolate(const std::vector<MessageFormats::DataPacket<T>> &packets, 
                 double targetSamplingRate = 100);
}
#endif
