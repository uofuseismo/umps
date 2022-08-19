#ifndef UMPS_AUTHENTICATION_AUTHENTICATOR_HPP
#define UMPS_AUTHENTICATION_AUTHENTICATOR_HPP
#include <string>
#include "umps/authentication/enums.hpp"
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Authentication::Certificate
{
 class Keys;
 class UserNameAndPassword;
}
namespace UMPS::Authentication
{
/// @class Authenticator "authenticator.hpp" "umps/messaging/authentication/authenticator.hpp"
/// @brief This the base class that defines an authenticator.  
/// @note This is based on the pyzmq implementation in base.py
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IAuthenticator
{
public:
    /// @result Determines if the given IP address is blacklisted.
    [[nodiscard]] virtual std::pair<std::string, std::string> isBlacklisted(const std::string &address) const noexcept = 0;
    /// @result Determines if the given IP address is whitelisted.
    [[nodiscard]] virtual std::pair<std::string, std::string> isWhitelisted(const std::string &address) const noexcept = 0;
    /// @result The minimum user privileges for this authenticator.
    [[nodiscard]] virtual UserPrivileges getMinimumUserPrivileges() const noexcept = 0;
    /// @result Determines if the given username and password are allowed.
    /// @result result.first is the status code where "200" means okay,
    ///         "400" means a client error - i.e., invalid credentials,
    ///         and "500" indicates a server error.
    ///         result.second is the corresonding message to return via the
    ///         ZeroMQ Authentication Protocol.
    [[nodiscard]] virtual std::pair<std::string, std::string> isValid(
        const Certificate::UserNameAndPassword &credentials) const noexcept = 0;
    /// @result Determines if the given keys are valid.
    /// @result result.first is the status code where "200" means okay,
    ///         "400" means a client error - i.e., invalid credentials,
    ///         and "500" indicates a server error.
    ///         result.second is the corresonding message to return via the
    ///         ZeroMQ Authentication Protocol.
    [[nodiscard]] virtual std::pair<std::string, std::string> isValid(
        const Certificate::Keys &keys) const noexcept = 0;
    /// @result The okay status code to check against.
    [[nodiscard]] static std::string okayStatus() noexcept;
    /// @result An OK message to pass back when validation succeeds.
    [[nodiscard]] static std::string okayMessage() noexcept;
    /// @result Client error status code to check against.
    [[nodiscard]] static std::string clientErrorStatus() noexcept;
    /// @result Server error status code to check against.
    [[nodiscard]] static std::string serverErrorStatus() noexcept;
};
}
#endif
