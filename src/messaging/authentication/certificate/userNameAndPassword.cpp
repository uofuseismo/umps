#include <array>
#include <string>
#ifndef NDEBUG
#include <cassert>
#endif
#include <sodium/crypto_pwhash.h>
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"

using namespace UMPS::Messaging::Authentication::Certificate;

namespace
{

int hashLevelToIndex(const HashLevel level)
{
    if (level == HashLevel::INTERACTIVE){return 0;}
    if (level == HashLevel::MODERATE){return 1;}
#ifndef NDEBUG
    assert(level == HashLevel::SENSITIVE);
#endif
    return 2;
}

/// @brief This is utility for storing a password by first.
/// @param[in] password   The plain text password to convert to a hashed
///                       password.
/// @param[in] opslimit   Limits the operations.  This will take about
///                       3 seconds on 
/// @param[in] opslimit   Controls the max amount of computations performed
///                       by libsodium.
/// @param[in] memlimit   Controls the max amount of RAM libsodium will use.
/// @result The corresponding hashed string to store in a database.
/// @note The default algorithm will take about 3.5 seconds an a 2.8 GHz
///       Core i7 CPU and require ~1 Gb of RAM.
std::string pwhashString(
    const std::string &password,
    unsigned long long opslimit = crypto_pwhash_OPSLIMIT_SENSITIVE,
    unsigned long long memlimit = crypto_pwhash_MEMLIMIT_SENSITIVE)
{
    std::array<char, crypto_pwhash_STRBYTES> work;
    std::fill(work.begin(), work.end(), '\0');
    auto rc = crypto_pwhash_str(work.data(),
                                password.c_str(), password.size(),
                                opslimit, memlimit);
    if (rc != 0)
    {
        auto errmsg = "Failed to hash string.  Likely hit memory limit";
        throw std::runtime_error(errmsg);
    }
    std::string hashedPassword{work.data()};
    return hashedPassword;
}
}

class UserNameAndPassword::UserNameAndPasswordImpl
{
public:
    std::string mUserName;
    std::string mPassword;
    std::array<std::string, 3> mHashedPassword;
    std::array<bool, 3> mHaveHashedPassword{false, false, false};
    HashLevel mHashLevel = HashLevel::SENSITIVE;
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
    for (auto &pwd : pImpl->mHashedPassword)
    {
        pwd.clear();
    }
    pImpl->mHashLevel = HashLevel::SENSITIVE;
    pImpl->mHaveUserName = false;
    pImpl->mHavePassword = false;
    std::fill(pImpl->mHaveHashedPassword.begin(), 
              pImpl->mHaveHashedPassword.end(),
              false);
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
    for (auto &pwd : pImpl->mHashedPassword)
    {
        pwd.clear();
    }
    std::fill(pImpl->mHaveHashedPassword.begin(), 
              pImpl->mHaveHashedPassword.end(),
              false);
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

std::string UserNameAndPassword::getHashedPassword(
    const HashLevel level) const
{
    if (!havePassword()){throw std::runtime_error("Password not set");}
    auto idx = hashLevelToIndex(level);
    if (pImpl->mHaveHashedPassword[idx]){return pImpl->mHashedPassword[idx];}
    auto password = getPassword();
    if (level == HashLevel::SENSITIVE)
    {
        pImpl->mHashedPassword[idx]
            = pwhashString(password,
                           crypto_pwhash_OPSLIMIT_SENSITIVE,
                           crypto_pwhash_MEMLIMIT_SENSITIVE);
    }
    else if (level == HashLevel::INTERACTIVE)
    {
        pImpl->mHashedPassword[idx] 
            = pwhashString(password,
                           crypto_pwhash_OPSLIMIT_INTERACTIVE,
                           crypto_pwhash_MEMLIMIT_INTERACTIVE);
    }
    else //if (level == SENSITIVE)
    {
#ifndef NDEBUG
        assert(level == HashLevel::MODERATE);
#endif
        pImpl->mHashedPassword[idx] 
            = pwhashString(password,
                           crypto_pwhash_OPSLIMIT_MODERATE,
                           crypto_pwhash_MEMLIMIT_MODERATE);
    }
    pImpl->mHaveHashedPassword[idx] = true;
    return pImpl->mHashedPassword[idx];
}
