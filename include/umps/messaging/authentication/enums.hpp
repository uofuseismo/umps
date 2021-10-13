#ifndef UMPS_MESSAGING_AUTHENTICATION_ENUMS_HPP
#define UMPS_MESSAGING_AUTHENTICATION_ENUMS_HPP
namespace UMPS::Messaging::Authentication
{
/// @class SecurityLevel "enums.hpp" "umps/messaging/authentication/enums.hpp"
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
/// @class ValidationResult "enums.hpp" "umps/messaging/authentication/enums.hpp"
/// @brief Defines the authenticator result and, if validation fails, 
///        the reason for failure.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ValidationResult
{
    ALLOWED,            /*!< Access granted to user. */
    BLACKLISTED,        /*!< Access denied.  The user's IP has been blacklisted. */ 
    INVALID_USER,       /*!< Access denied.  The provided username cannot be found. */
    INVALID_PASSWORD,   /*!< Access denied.  The password for this user is invalid. */ 
    INVALID_PUBLIC_KEY, /*!< Access denied.  The user's public key is invalid. */
    ALGORITHM_FAILURE   /*!< Access denied.  The validator failed. */
};
/// @class UserPrivileges "enums.hpp" "umps/messaging/authentication/enums.hpp"
/// @brief Defines the user's privileges.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class UserPrivileges
{
    READ_ONLY,     /*!< The user has read-only permissions. */
    READ_WRITE,    /*!< The user has read-write permissions.  */
    ADMINISTRATOR  /*!< The user has administrative permissions. */
};
}
#endif
