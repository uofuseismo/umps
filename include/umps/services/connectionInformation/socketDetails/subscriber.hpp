#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_SUBSCRIBER_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_SUBSCRIBER_HPP
#include <memory>
#include "umps/services/connectionInformation/enums.hpp"
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
/// @class Subscriber "subscriber.hpp" "umps/services/connectionInformation/socketDetails/subscriber.hpp"
/// @brief Defines how a client would bind to this subscriber socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Subscriber
{
public:
    /// @name Constructors
    /// @{ 
    /// @brief Constructor.
    Subscriber();
    /// @brief Copy constructor.
    /// @param[in] socket  The socket from which to initialize this class.
    Subscriber(const Subscriber &socket); 
    /// @brief Move constructor.
    /// @param[in,out] socket  The socket from which to initialize this class.
    ///                        On exit, socket's behavior is undefined.
    Subscriber(Subscriber &&socket) noexcept;
    /// @}
    
    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] socket  The socket to copy to this.
    /// @result A deep copy of the input socket connection details. 
    Subscriber& operator=(const Subscriber &socket);
    /// @brief Move assignment operator.
    /// @param[in,out] socket  The socket whose memory will be moved to this.
    ///                        On exit, socket's behavior is undefined.
    /// @result The memory from socket moved to this.
    Subscriber& operator=(Subscriber &&socket) noexcept;
    /// @}
 
    /// @brief The address to which to bind.
    void setAddress(const std::string &address);
    /// @result The address to which to bind to the socket.
    /// @throws std::invalid_argument if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address is set.
    [[nodiscard]] bool haveAddress() const noexcept;

    /// @result Defines whether peers should connect or bind.
    static ConnectOrBind connectOrBind() noexcept;
    /// @result The socket type.
    static SocketType getSocketType() noexcept;

    /// @name Destructors
    /// @{
    /// @brief Reset the class and release all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~Subscriber();
    /// @}
private:
    class SubscriberImpl;
    std::unique_ptr<SubscriberImpl> pImpl;
};
}
#endif
