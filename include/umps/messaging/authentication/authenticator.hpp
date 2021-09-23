#ifndef UMPS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#define UMPS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#include <memory>
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Messaging::Authentication
{
class Certificate;
class Authenticator
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Authenticator();
    /// @}

    /// @name Destctructor
    ~Authenticator();
private:
    class AuthenticatorImpl;
    std::unique_ptr<AuthenticatorImpl> pImpl;
};
}
#endif
