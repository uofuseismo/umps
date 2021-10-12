#ifndef UMPS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#define UMPS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Messaging::Authentication::Certificate
{
 class Keys;
 class UserNameAndPassword;
}
namespace UMPS::Messaging::Authentication
{
/// @class Authenticator "authenticator.hpp" "umps/messaging/authentication/authenticator.hpp"
/// @brief This the base class that defines an authenticator.  
/// @note This is based on the pyzmq implementation in base.py
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IAuthenticator
{
public:
    /// @result Determines if the given IP address is blacklisted.
    [[nodiscard]] virtual ValidationResult isBlacklisted(const std::string &address) const noexcept = 0;
    /// @result Determines if the given IP address is whitelisted.
    [[nodiscard]] virtual ValidationResult isWhitelisted(const std::string &address) const noexcept = 0;
    /// @result Determines if the given username and password are allowed.
    [[nodiscard]] virtual ValidationResult isValid(
        const Certificate::UserNameAndPassword &credentials) const noexcept = 0;
    /// @result Determines if the given keys are valid.
    [[nodiscard]] virtual ValidationResult isValid(
        const Certificate::Keys &keys) const noexcept = 0;
};
}
#endif
