#ifndef PRIVATE_APPLICATIONS_PACKETCACHE_HPP
#define PRIVATE_APPLICATIONS_PACKETCACHE_HPP
#include <string>
#include <algorithm>
#include "private/isEmpty.hpp"
#include "umps/messageFormats/dataPacket.hpp"
namespace
{
/// @result The name network, station, channel, and location code combined into
///         a single name - e.g., UU.FORK.HHZ.01.
[[maybe_unused]] [[nodiscard]] 
std::string makeName(const std::string &network,
                     const std::string &station,
                     const std::string &channel,
                     const std::string &locationCode)
{
    auto name = network + "." + station + "."  + channel + "." + locationCode;
    return name;
}
/// @result Converts the packet network, station, channel, and location code 
///         into a single name - e.g., UU.FORK.HHZ.01.
template<typename T>
[[maybe_unused]] [[nodiscard]]
std::string makeName(const UMPS::MessageFormats::DataPacket<T> &packet)
{
    return makeName(packet.getNetwork(), packet.getStation(),
                    packet.getChannel(), packet.getLocationCode());

}
/// @result True indicates all necessary information to set a data packet
///         in the circular buffer is present.
template<typename T>
[[maybe_unused]] [[nodiscard]]
bool isValidPacket(const UMPS::MessageFormats::DataPacket<T> &packet)
{
    if (!packet.haveNetwork())
    {
        return false;
    }
    if (!packet.haveStation())
    {
        return false;
    }
    if (!packet.haveChannel())
    {
        return false;
    }
    if (!packet.haveLocationCode())
    {
        return false;
    }
    if (!packet.haveSamplingRate())
    {
        return false;
    }
    if (packet.getNumberOfSamples() < 1)
    {
        return false;
    }
    return true;
}
}
#endif
