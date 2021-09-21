#ifndef URTS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#define URTS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#include <memory>
namespace URTS::Messaging::Authentication
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
