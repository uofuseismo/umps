#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_PROXY_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_PROXY_HPP
#include <memory>
#include "umps/services/connectionInformation/enums.hpp"
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
 //class Publisher;
 //class Subscriber;
 //class Request;
 //class Router;
 class XPublisher;
 class XSubscriber;
}
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
/// @class Proxy "proxy.hpp" "umps/services/connectionInformation/socketDetails/proxy.hpp"
/// @brief Defines how a client would connect to this proxy socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Proxy
{
public:
    /// @name Constructors
    /// @{ 
    /// @brief Constructor.
    Proxy();
    /// @brief Copy constructor.
    /// @param[in] socket  The socket from which to initialize this class.
    Proxy(const Proxy &socket); 
    /// @brief Move constructor.
    /// @param[in,out] socket  The socket from which to initialize this class.
    ///                        On exit, socket's behavior is undefined.
    Proxy(Proxy &&socket) noexcept;
    /// @}
    
    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] socket  The socket to copy to this.
    /// @result A deep copy of the input socket connection details. 
    Proxy& operator=(const Proxy &socket);
    /// @brief Move assignment operator.
    /// @param[in,out] socket  The socket whose memory will be moved to this.
    ///                        On exit, socket's behavior is undefined.
    /// @result The memory from socket moved to this.
    Proxy& operator=(Proxy &&socket) noexcept;
    /// @}

    /// @brief Sets the socket pair for this proxy.
    /// @param[in] socketPair  socketPair.first is the frontend (where data
    ///                        comes in) and socketPair.end is the backend
    ///                        (where data goes out).  
    /// @throws std::invalid_argument if the socketPair.first or
    ///         socketPair.second does not have an address.
    void setSocketPair(const std::pair<XSubscriber, XPublisher> &socketPair);
    /// @result True indicates that the socket pair was set.
    [[nodiscard]] bool haveSocketPair() const noexcept;
    /// @result The socket type of the frontend.
    /// @throws std::runtime_error if \c haveSocketPair() is false.
    [[nodiscard]] SocketType getFrontendSocketType() const;
    /// @result The socket type of the backend.
    /// @throws std::runtime_error if \c haveSocketPair() is false.
    [[nodiscard]] SocketType getBackendSocketType() const;
    /// @result The socket information for the frontend.
    [[nodiscard]] XSubscriber getXSubscriberFrontend() const;
    /// @result The socket information for the backend.
    [[nodiscard]] XPublisher  getXPublisherBackend() const;

    /// @result The address of the frontend socket.
    /// @throws std::runtime_error if \c haveSocketPair() is false.
    [[nodiscard]] std::string getFrontendAddress() const;
    /// @result The address of the backend socket.
    /// @throws std::runtime_error if \c haveSocketPair() is false.
    [[nodiscard]] std::string getBackendAddress() const;
    /// @}

    /// @result The socket type.
    [[nodiscard]] static SocketType getSocketType() noexcept;

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~Proxy();
    /// @}
private:
    class ProxyImpl;
    std::unique_ptr<ProxyImpl> pImpl;
};
}
#endif
