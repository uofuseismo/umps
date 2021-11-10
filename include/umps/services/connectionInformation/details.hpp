#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_DETAILS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_DETAILS_HPP
#include <memory>
#include <string>
#include "umps/services/connectionInformation/enums.hpp"
#include "umps/messaging/authentication/enums.hpp"
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
 class Proxy;
 class Publisher;
 class Request;
 class Router;
 class Subscriber;
 class XPublisher;
 class XSubscriber;
}
namespace UMPS::Services::ConnectionInformation
{
/// @class Details "details.hpp" "umps/services/connectionInformation/details.hpp"
/// @brief This class provides the connection requisite details to establish
///        a connection to a service or broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Details
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Details();
    /// @brief Copy constructor.
    /// @param[in] details  Creates this class from the given details.
    Details(const Details &details);
    /// @brief Move constructor.
    /// @param[in,out] response  Creates this class from the given response.
    ///                          On exit, response's behavior is undefined.
    Details(Details &&details) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move constructor.
    /// @param[in] details  The details to copy this.
    /// @result A deep copy of the input details.
    Details& operator=(const Details &details);
    /// @brief Move constructor.
    /// @param[in,out] details  The details whose memory will be moved to this.
    ///                         On exit, details's behavior is undefined.
    /// @result The memory from details moved to this.
    Details& operator=(Details &&details) noexcept;
    /// @}

    /// @brief Sets the name of the broadcast (or service).
    /// @param[in] name  The name of the broadcast or service.
    /// @throws std::invalid_argument if the name is empty.
    void setName(const std::string &name); 
    /// @result The name of the service.
    /// @throws std::runtime_error if \c haveName() is false.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates that the service name is defined.
    [[nodiscard]] bool haveName() const noexcept;
   

    /// @name Socket Details
    /// @{
    /// @brief Sets the publisher socket connection details.
    /// @param[in] socket  The publisher socket connection details.
    /// @throws std::invalid_argument if \c socket.haveAddress() is false.
    void setSocketDetails(const SocketDetails::Publisher &socket);
    /// @brief Sets the subscriber socket connection details.
    /// @param[in] socket  The subscriber socket connection details.
    /// @throws std::invalid_argument if \c socket.haveAddress() is false.
    void setSocketDetails(const SocketDetails::Subscriber &socket);
    /// @brief Sets the request socket connection details.
    /// @param[in] socket  The request socket connection details.
    /// @throws std::invalid_argument if \c socket.haveAddress() is false.
    void setSocketDetails(const SocketDetails::Request &socket);
    /// @brief Sets the router socket connection details.
    /// @param[in] socket  The router socket connection details.
    /// @throws std::invalid_argument if \c socket.haveAddress() is false.
    void setSocketDetails(const SocketDetails::Router &socket);
    /// @brief Sets the extended publisher socket connection details.
    /// @param[in] socket  The extended publisher socket connection details.
    /// @throws std::invalid_argument if \c socket.haveAddress() is false.
    void setSocketDetails(const SocketDetails::XPublisher &socket);
    /// @brief Sets the extended subscriber socket connection details.
    /// @param[in] socket  The extended subscriber socket connection details.
    /// @throws std::invalid_argument if \c socket.haveAddress() is false.
    void setSocketDetails(const SocketDetails::XSubscriber &socket);
    /// @brief Sets the proxy pattern.
    /// @param[in] proxy   The socket details for the frontend and backend
    ///                    of the proxy.
    /// @throws std::invalid_argument if \c socket.haveSocketPair() is false.
    void setSocketDetails(const SocketDetails::Proxy &proxy);

    /// @result The publisher socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::PUBLISHER.
    SocketDetails::Publisher  getPublisherSocketDetails() const;
    /// @result The subscriber socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::SUBSCRIBER.
    SocketDetails::Subscriber getSubscriberSocketDetails() const;
    /// @result The request socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::REQUEST.
    SocketDetails::Request getRequestSocketDetails() const;
    /// @result The router socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::ROUTER.
    SocketDetails::Router getRouterSocketDetails() const;
    /// @result The publisher socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::XPUBLISHER.
    SocketDetails::XPublisher  getXPublisherSocketDetails() const;
    /// @result The subscriber socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::XSUBSCRIBER.
    SocketDetails::XSubscriber getXSubscriberSocketDetails() const;
    /// @result The proxy socket details.
    /// @throws std::runtime_error if \c getSocketType() does not equal
    ///         SocketType::PROXY.
    SocketDetails::Proxy getProxySocketDetails() const;
    /// @result The socket type.
    /// @note By default this is unknown and will obtain value from the last
    ///       call to \c setSocketDetails().
    SocketType getSocketType() const noexcept;
    /// @}

    /// @brief Sets the connection string, e.g., "tcp://127.0.0.1:8080".
    /// @param[in] connectionString  The connection string.
    /// @throws std::invalid_argument if connectionString is empty. 
    //void setConnectionString(const std::string &connectionString);    
    /// @result The connection string.
    /// @throws std::runtime_error if \c haveConnectionString() is false.
    //[[nodiscard]] std::string getConnectionString() const;
    /// @result True indicates that the connection string is set. 
    //[[nodiscard]] bool haveConnectionString() const noexcept;

    /// @brief Sets the connection type - e.g., a broadcast or service.
    /// @param[in] connectionType  The connection type.
    void setConnectionType(ConnectionType connectionType) noexcept;
    /// @result The connection type.
    [[nodiscard]] ConnectionType getConnectionType() const;
    /// @result True indicates the connection type was set.
    [[nodiscard]] bool haveConnectionType() const noexcept; 

    /// @brief The required privileges to connect.
    /// @param[in] privileges  The privileges required to connect.
    void setUserPrivileges(const UMPS::Messaging::Authentication::UserPrivileges privileges) noexcept;
    /// @result The requisite privileges to connect.
    [[nodiscard]] UMPS::Messaging::Authentication::UserPrivileges getUserPrivileges() const noexcept;

    /// @brief Sets the security level required to establish a connection.
    /// @param[in] securityLevel  The requisite security level.
    void setSecurityLevel(UMPS::Messaging::Authentication::SecurityLevel securityLevel) noexcept;
    /// @result The security level required to establish a connection.
    [[nodiscard]] UMPS::Messaging::Authentication::SecurityLevel getSecurityLevel() const noexcept;

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor
    ~Details();
    /// @}
private:
    class DetailsImpl;
    std::unique_ptr<DetailsImpl> pImpl;
};
}
#endif
