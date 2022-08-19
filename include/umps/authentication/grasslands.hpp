#ifndef UMPS_AUTHENTICATION_GRASSLANDS_HPP
#define UMPS_AUTHENTICATION_GRASSLANDS_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
#include "umps/authentication/authenticator.hpp"
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
/// @class Grasslands "grasslands.hpp" "umps/messaging/authentication/grasslands.hpp"
/// @brief This is a default authenticator that will allow all connections.
/// @note If you are serious about validating users then do not use this class.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Grasslands : public IAuthenticator
{
public:
    /// @brief Constructor.
    Grasslands();
    /// @brief Constructor with a given logger.
    explicit Grasslands(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Destructor.
    virtual ~Grasslands();
    /// @result The minimum user privileges.
    [[nodiscard]] virtual UserPrivileges getMinimumUserPrivileges() const noexcept override final;
    /// @result Determines if the given IP address is blacklisted.
    [[nodiscard]] virtual std::pair<std::string, std::string> isBlacklisted(
        const std::string &address) const noexcept override final;
    /// @result Determines if the given IP address is whitelisted.
    [[nodiscard]] virtual std::pair<std::string, std::string> isWhitelisted(
        const std::string &) const noexcept override final;
    /// @result Determines if the given username and password are allowed.
    [[nodiscard]] virtual std::pair<std::string, std::string> isValid(
        const Certificate::UserNameAndPassword &) const noexcept override final;
    /// @result Determines if the given keys are valid.
    [[nodiscard]] virtual std::pair<std::string, std::string> isValid(
        const Certificate::Keys &) const noexcept override final;
private:
    class GrasslandsImpl;
    std::unique_ptr<GrasslandsImpl> pImpl;
};
}
#endif
