#ifndef UMPS_PROXYSERVICES_PACKETCACHE_ENUMS_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_ENUMS_HPP
#include <memory>
namespace UMPS::ProxyServices::PacketCache
{
/// @brief These are return codes from the packet cache.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ReturnCode
{
    SUCCESS = 0,              /*!< No errors were detected; the request was succesful. */
    NO_SENSOR = 1,            /*!< The data for the requested sensor
                                   (Network, Station, Channel, Location code) does not exist. */
    INVALID_MESSAGE_TYPE = 2, /*!< The received message type is not supported. */
    INVALID_MESSAGE = 3,      /*!< The message could not be parsed. */
    INVALID_TIME_QUERY = 4,   /*!< The time query parameters are invalid. */ 
    ALGORITHM_FAILURE = 5     /*!< An internal error was detected. */
};
}
#endif
