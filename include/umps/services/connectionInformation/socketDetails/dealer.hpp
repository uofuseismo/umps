#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_DEALER_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_SOCKETDETAILS_DEALER_HPP
#include <memory>
#include "umps/services/connectionInformation/enums.hpp"
namespace UMPS::Services::ConnectionInformation::SocketDetails
{
/// @class Dealer "dealer.hpp" "umps/services/connectionInformation/socketDetails/dealer.hpp"
/// @brief Defines how a client would connect to this dealer socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Dealer
{
public:
    /// @name Constructors
    /// @{ 

    /// @brief Constructor.
    Dealer();
    /// @brief Copy constructor.
    /// @param[in] socket  The socket from which to initialize this class.
    Dealer(const Dealer &socket); 
    /// @brief Move constructor.
    /// @param[in,out] socket  The socket from which to initialize this class.
    ///                        On exit, socket's behavior is undefined.
    Dealer(Dealer &&socket) noexcept;
    /// @}
    
    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] socket  The socket to copy to this.
    /// @result A deep copy of the input socket connection details. 
    Dealer& operator=(const Dealer &socket);
    /// @brief Move assignment operator.
    /// @param[in,out] socket  The socket whose memory will be moved to this.
    ///                        On exit, socket's behavior is undefined.
    /// @result The memory from socket moved to this.
    Dealer& operator=(Dealer &&socket) noexcept;
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
    ~Dealer();
    /// @}
private:
    class DealerImpl;
    std::unique_ptr<DealerImpl> pImpl;
};
}
#endif
