#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_XPUBLISHER_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_XPUBLISHER_HPP
#include <memory>
#include "umps/services/connectionInformation/enums.hpp"
#include "umps/authentication/enums.hpp"
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
/// @class XPublisher "xPublisher.hpp" "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
/// @brief Defines how a client would connect to this extended publisher socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class XPublisher
{
public:
    /// @name Constructors
    /// @{ 

    /// @brief Constructor.
    XPublisher();
    /// @brief Copy constructor.
    /// @param[in] socket  The socket from which to initialize this class.
    XPublisher(const XPublisher &socket); 
    /// @brief Move constructor.
    /// @param[in,out] socket  The socket from which to initialize this class.
    ///                        On exit, socket's behavior is undefined.
    XPublisher(XPublisher &&socket) noexcept;
    /// @}
    
    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] socket  The socket to copy to this.
    /// @result A deep copy of the input socket connection details. 
    XPublisher& operator=(const XPublisher &socket);
    /// @brief Move assignment operator.
    /// @param[in,out] socket  The socket whose memory will be moved to this.
    ///                        On exit, socket's behavior is undefined.
    /// @result The memory from socket moved to this.
    XPublisher& operator=(XPublisher &&socket) noexcept;
    /// @}
 
    /// @brief The address to which to connect.
    void setAddress(const std::string &address);
    /// @result The address to which to connect to the socket.
    /// @throws std::invalid_argument if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address is set.
    [[nodiscard]] bool haveAddress() const noexcept;

    /// @brief Sets the security level.
    void setSecurityLevel(const UMPS::Authentication::SecurityLevel level) noexcept;
    /// @result The security level.
    /// @note The default is grasslands.
    [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept;

    /// @brief Defines whether peers should connect or bind.
    /// @param[in] connectOrBind  Defines if peers should connect or bind
    ///                           to interact with this socket.
    void setConnectOrBind(ConnectOrBind connectOrBind) noexcept;
    /// @result Defines whether peers should connect or bind.
    [[nodiscard]] ConnectOrBind getConnectOrBind() const noexcept;
    /// @result The socket type.
    [[nodiscard]] static SocketType getSocketType() noexcept;

    /// @name Destructors
    /// @{

    /// @brief Reset the class and release all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~XPublisher();
    /// @}
private:
    class XPublisherImpl;
    std::unique_ptr<XPublisherImpl> pImpl;
};
}
#endif
