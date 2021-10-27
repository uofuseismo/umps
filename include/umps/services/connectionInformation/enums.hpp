#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_ENUMS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_ENUMS_HPP
#include <memory>
namespace UMPS::Services::ConnectionInformation
{
/// @brief Defines the connection type.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ConnectionType
{
    SERVICE,   /*!< The connection is to a service.  This will be a 
                    generalization of a request-reply socket.  */
    BROADCAST  /*!< The connection is to a broadcast.  This will be 
                    generalization of a publisher-subscribe socket. */
};
/// @brief Defines the return codes for the service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ReturnCode
{
    SUCCESS = 0,           /*!< No errors were detected; the request was succesful. */
    NO_ITEM = 1,           /*!< The desired item could not be found. */
    INVALID_MESSAGE = 2,   /*!< The message could not be parsed. */
    ALGORITHM_FAILURE = 3  /*!< An internal counting error was detected.
                                The returned value should not be trusted to be unique. */
};
}
#endif
