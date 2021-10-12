#ifndef UMPS_MESSAGING_AUTHENTICATION_GRASSLANDS_HPP
#define UMPS_MESSAGING_AUTHENTICATION_GRASSLANDS_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
#include "umps/messaging/authentication/authenticator.hpp"
namespace UMPS::Messaging::Authentication::Certificate
{
 class Keys;
 class UserNameAndPassword;
}
namespace UMPS::Messaging::Authentication
{
/// @class Grasslands "grasslands.hpp" "umps/messaging/authentication/grasslands.hpp"
/// @brief This is a default authenticator that will allow all connections.
/// @note If you are serious about validating users then do not use this class.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Grasslands : public IAuthenticator
{
public:
    /// @brief Destructor.
    virtual ~Grasslands() = default;
    /// @result Determines if the given IP address is blacklisted.
    [[nodiscard]] virtual ValidationResult isBlacklisted(
        const std::string &address) const noexcept
    {
        return ValidationResult::ALLOWED;
    }
    /// @result Determines if the given IP address is whitelisted.
    [[nodiscard]] virtual ValidationResult isWhitelisted(
        const std::string &) const noexcept
    { 
        return ValidationResult::ALLOWED; 
    } 
    /// @result Determines if the given username and password are allowed.
    [[nodiscard]] virtual ValidationResult isValid(
        const Certificate::UserNameAndPassword &) const noexcept
    { 
        return ValidationResult::ALLOWED; 
    }
    /// @result Determines if the given keys are valid.
    [[nodiscard]] virtual ValidationResult isValid(
        const Certificate::Keys &) const noexcept
    { 
        return ValidationResult::ALLOWED; 
    }
};
}
#endif
