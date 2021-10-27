#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_DETAILS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_DETAILS_HPP
#include <memory>
#include <string>
#include "umps/services/connectionInformation/enums.hpp"
#include "umps/messaging/authentication/enums.hpp"
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
   
    /// @brief Sets the connection string, e.g., tcp://127.0.0.1:8080
    /// @param[in] connectionString  The connection string.
    /// @throws std::invalid_argument if connectionString is empty. 
    void setConnectionString(const std::string &connectionString);    
    /// @result The connection string.
    /// @throws std::runtime_error if \c haveConnectionString() is false.
    [[nodiscard]] std::string getConnectionString() const;
    /// @result True indicates that the connection string is set. 
    [[nodiscard]] bool haveConnectionString() const noexcept;

    /// @brief Sets the security level required to establish a connection.
    void setSecurityLevel(UMPS::Messaging::Authentication::SecurityLevel level) noexcept;
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
