#ifndef UMPS_AUTHENTICATION_ENUMS_HPP
#define UMPS_AUTHENTICATION_ENUMS_HPP
namespace UMPS::Authentication
{
/// @class SecurityLevel "enums.hpp" "umps/authentication/enums.hpp"
/// @brief Defines the security level of a connection.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Authentication_SecurityLevels
enum class SecurityLevel
{
    Grasslands = 0,  /*!< No validation. */
    Strawhouse = 1,  /*!< Server validates client's IP address. */
    Woodhouse = 2,   /*!< Server validates client's IP address, username,
                          and password. */
    Stonehouse = 3   /*!< Server validates client's IP address and client's
                          public key. */
    //Ironhouse =4   /*!<  Server validates client's IP address and client's 
    //                     public key and client validate's server's public key. */
};
/// @class UserPrivileges "enums.hpp" "umps/authentication/enums.hpp"
/// @brief Defines the user's privileges.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Authentication_UserPrivileges
enum class UserPrivileges
{
    ReadOnly = 0,      /*!< The user has read-only permissions. */
    ReadWrite = 1,     /*!< The user has read-write permissions.  */
    Administrator = 2  /*!< The user has administrative permissions. */
};
}
#endif
