#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_ENUMS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_ENUMS_HPP
#include <memory>
namespace UMPS::Services::ConnectionInformation
{
/// @brief Defines the connection type.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ConnectionType
{
    Service,   /*!< The connection is to a service.  This will be a 
                    generalization of a request-reply socket.  */
    Broadcast  /*!< The connection is to a broadcast.  This will be 
                    generalization of a publisher-subscribe socket. */
};
/// @brief Defines whether peers should bind or connect to access the socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ConnectOrBind
{
    Connect,  /*!< To connect to the socket peers connect.  */
    Bind      /*!< To connect to the socket peers must bind.  */
};
/// @brief This defines the socket type.  Note, ZeroMQ may have more socket
///        types than what is enumerated here.  These are simply the patterns
///        exposed in UMPS.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class SocketType
{
    Unknown,     /*!< The socket type is not specified. */
    Request,     /*!< This is a socket that receives requests. 
                      This will accept connections from a RESPONSE or
                      DEALER socket. */
    Dealer,      /*!< This is a socket that receives requests asynchronously.
                      This will accept connections from a ROUTER, RESPONSE, or
                      DEALER socket. */
    Reply,       /*!< This is a socket that replies to responses.
                      This will accept connections from a REQUEST or
                      ROUTER socket. */
    Response,    /*!< This is a socket that receives responses.
                      This will accept connections from a REQUEST
                      or DEALER socket. */
    Router,      /*!< This is a socket that receives responses asynchronously.
                      This will accept connections from a DEALER, REQUEST,
                      or ROUTER socket. */ 
    Publisher,   /*!< This is a publisher socket.
                      This will accept connections from a SUBSCRIBE or
                      XSUBSCRIBE socket. */ 
    XPublisher,  /*!< This is an extended publisher socket.
                      This will accept connections from a SUBSCRIBE or
                      XSUBSCRIBE socket. */ 
    Subscriber,  /*!< This is a subscription socket.
                      This will accept connections from a PUBLISH or
                      XPUBLISH socket. */
    XSubscriber, /*!< This is an extended subscription socket.
                      This accept connections from a PUBLISH or
                      XPUBLISH socket. */
    Proxy        /*!< A proxy really isn't a socket but a paradigm. 
                      It indicates that there could be a REQUEST-ROUTER
                      or XPUB-XSUB pattern.  For this socket type the
                      user will have to get the frontend and backend
                      connection details. */
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
