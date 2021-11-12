#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_PUBLISHER_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_PUBLISHER_HPP
#include <memory>
#include "umps/services/connectionInformation/enums.hpp"
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
/// @class Publisher "publisher.hpp" "umps/services/connectionInformation/socketDetails/publisher.hpp"
/// @brief Defines how a client would connect to this publisher socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Publisher
{
public:
    /// @name Constructors
    /// @{ 
    /// @brief Constructor.
    Publisher();
    /// @brief Copy constructor.
    /// @param[in] socket  The socket from which to initialize this class.
    Publisher(const Publisher &socket); 
    /// @brief Move constructor.
    /// @param[in,out] socket  The socket from which to initialize this class.
    ///                        On exit, socket's behavior is undefined.
    Publisher(Publisher &&socket) noexcept;
    /// @}
    
    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] socket  The socket to copy to this.
    /// @result A deep copy of the input socket connection details. 
    Publisher& operator=(const Publisher &socket);
    /// @brief Move assignment operator.
    /// @param[in,out] socket  The socket whose memory will be moved to this.
    ///                        On exit, socket's behavior is undefined.
    /// @result The memory from socket moved to this.
    Publisher& operator=(Publisher &&socket) noexcept;
    /// @}
 
    /// @brief The address to which to connect.
    void setAddress(const std::string &address);
    /// @result The address to which to connect to the socket.
    /// @throws std::invalid_argument if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address is set.
    [[nodiscard]] bool haveAddress() const noexcept;

    /// @result Defines whether peers should connect or bind.
    [[nodiscard]] static ConnectOrBind connectOrBind() noexcept;
    /// @result The socket type.
    [[nodiscard]] static SocketType getSocketType() noexcept;

    /// @name Destructors
    /// @{
    /// @brief Reset the class and release all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~Publisher();
    /// @}
private:
    class PublisherImpl;
    std::unique_ptr<PublisherImpl> pImpl;
};
}
#endif