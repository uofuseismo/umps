#ifndef UMPS_AUTHENTICATION_ENUMS_HPP
#define UMPS_AUTHENTICATION_ENUMS_HPP
namespace UMPS::Authentication
{
/// @class SecurityLevel "enums.hpp" "umps/authentication/enums.hpp"
/// @brief Defines the security level of a connection.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class SecurityLevel
{
    GRASSLANDS,  /*!< No validation. */
    STRAWHOUSE,  /*!< Server validates client's IP address. */
    WOODHOUSE,   /*!< Server validates client's IP address, username,
                      and password. */
    STONEHOUSE,  /*!< Server validates client's IP address and client's
                      public key. */
    IRONHOUSE    /*!< Server validates client's IP address and client's 
                      public key and client validate's server's public key. */
};
/// @class UserPrivileges "enums.hpp" "umps/authentication/enums.hpp"
/// @brief Defines the user's privileges.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class UserPrivileges
{
    READ_ONLY = 0,     /*!< The user has read-only permissions. */
    READ_WRITE = 1,    /*!< The user has read-write permissions.  */
    ADMINISTRATOR = 2  /*!< The user has administrative permissions. */
};
}
#endif
