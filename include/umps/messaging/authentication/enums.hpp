#ifndef UMPS_MESSAGING_AUTHENTICATION_ENUMS_HPP
#define UMPS_MESSAGING_AUTHENTICATION_ENUMS_HPP
namespace UMPS::Messaging::Authentication
{
/// @class SecurityLevel "enums.hpp" "umps/messaging/authentication/enums.hpp"
/// @brief Defines the security level of a connection.
/// @copyright Ben Baker (University of Utah) distributed under the MIT License.
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
}
#endif
