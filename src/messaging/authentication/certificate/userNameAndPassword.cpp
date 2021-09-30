#include <string>
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"

using namespace UMPS::Messaging::Authentication::Certificate;

class UserNameAndPassword::UserNameAndPasswordImpl
{
public:
    std::string mUserName;
    std::string mPassword;
    bool mHaveUserName = false;
    bool mHavePassword = false;
};

/// C'tor
UserNameAndPassword::UserNameAndPassword() :
    pImpl(std::make_unique<UserNameAndPasswordImpl> ())
{
}

/// Copy c'tor
UserNameAndPassword::UserNameAndPassword(const UserNameAndPassword &plainText)
{
    *this = plainText;
}

/// Move c'tor
UserNameAndPassword::UserNameAndPassword(
    UserNameAndPassword &&plainText) noexcept
{
    *this = std::move(plainText);
}

/// Copy assignment
UserNameAndPassword& UserNameAndPassword::operator=(
    const UserNameAndPassword &plainText)
{
    if (&plainText == this){return *this;}
    pImpl = std::make_unique<UserNameAndPasswordImpl> (*plainText.pImpl);
    return *this;
}

/// Move assignment
UserNameAndPassword& UserNameAndPassword::operator=(
   UserNameAndPassword &&plainText) noexcept
{
    if (&plainText == this){return *this;}
    pImpl = std::move(plainText.pImpl);
    return *this;
}

/// Destructor
UserNameAndPassword::~UserNameAndPassword() = default;

/// Reset class
void UserNameAndPassword::clear() noexcept
{
    pImpl->mUserName.clear();
    pImpl->mPassword.clear();
    pImpl->mHaveUserName = false;
    pImpl->mHavePassword = false;
}

/// User name
void UserNameAndPassword::setUserName(const std::string &name) noexcept
{
    pImpl->mUserName = name;
    pImpl->mHaveUserName = true;
}

std::string UserNameAndPassword::getUserName() const
{
    if (!haveUserName()){throw std::runtime_error("User name not set");}
    return pImpl->mUserName;
}

bool UserNameAndPassword::haveUserName() const noexcept
{
    return pImpl->mHaveUserName;
}

/// Password
void UserNameAndPassword::setPassword(const std::string &password) noexcept
{
    pImpl->mPassword = password;
    pImpl->mHavePassword = true;
}

std::string UserNameAndPassword::getPassword() const
{
    if (!havePassword()){throw std::runtime_error("Password not set");}
    return pImpl->mPassword;
}

bool UserNameAndPassword::havePassword() const noexcept
{
    return pImpl->mHavePassword;
}
