#ifndef PYUMPS_AUTHENTICATION_USERNAMEANDPASSWORD_HPP
#define PYUMPS_AUTHENTICATION_USERNAMEANDPASSWORD_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "umps/authentication/enums.hpp"
namespace UMPS::Authentication::Certificate
{
 class UserNameAndPassword;
}
namespace PUMPS::Authentication
{
class UserNameAndPassword
{
public:
    UserNameAndPassword();
    UserNameAndPassword(const UserNameAndPassword &credentials);
    UserNameAndPassword(const UMPS::Authentication::Certificate::UserNameAndPassword &credentials);
    UserNameAndPassword(UserNameAndPassword &&credentials) noexcept;
    UserNameAndPassword& operator=(const UserNameAndPassword &credentials);
    UserNameAndPassword& operator=(UserNameAndPassword &&credentials) noexcept;
    UserNameAndPassword& operator=(const UMPS::Authentication::Certificate::UserNameAndPassword &credentials);
    UMPS::Authentication::Certificate::UserNameAndPassword getNativeClass() const noexcept;
    ~UserNameAndPassword();

private:
    std::unique_ptr<UMPS::Authentication::Certificate::UserNameAndPassword> pImpl;
};
}
#endif

