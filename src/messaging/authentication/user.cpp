#include <ostream>
#include <iostream>
#include <string>
#include <sodium/crypto_pwhash.h>
#include <nlohmann/json.hpp>
#include "umps/messaging/authentication/user.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Messaging::Authentication::User"

using namespace UMPS::Messaging::Authentication;

namespace
{
nlohmann::json toJSONObject(const User &user)
{
    nlohmann::json obj;
    obj["MessageType"] = user.getMessageType();
    if (user.haveName())
    {
         obj["Name"] = user.getName();
    }
    else
    {
         obj["Name"] = nullptr;
    }
    if (user.haveEmail())
    {
        obj["Email"] = user.getEmail();
    }
    else
    {
        obj["Email"] = nullptr;
    }
    if (user.haveHashedPassword())
    {
        obj["HashedPassword"] = user.getHashedPassword();
    }
    else
    {
        obj["HashedPassword"] = nullptr;
    }
    if (user.havePublicKey())
    {
        obj["PublicKey"] = user.getPublicKey();
    }
    else
    {
        obj["PublicKey"] = nullptr;
    }
    if (user.haveIdentifier())
    {
        obj["Identifier"] = user.getIdentifier();
    }
    else
    {
        obj["Identifier"] = nullptr;
    }
    obj["Privileges"] = static_cast<int> (user.getPrivileges());
    return obj;
}

User objectToUser(const nlohmann::json obj)
{
    User user;
    if (obj["MessageType"] != user.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    std::string name = obj["Name"].get<std::string> ();
    if (!name.empty()){user.setName(name);}
    std::string email = obj["Email"].get<std::string> ();
    if (!email.empty()){user.setEmail(email);}
    std::string password = obj["HashedPassword"].get<std::string> ();
    if (!password.empty()){user.setHashedPassword(password);}
    std::string publicKey = obj["PublicKey"].get<std::string> ();
    if (static_cast<int> (publicKey.length()) == 40)
    {
        user.setPublicKey(publicKey);
    }
    if (obj["Identifier"] != nullptr)
    {
        user.setIdentifier(obj["Identifier"].get<int> ());
    } 
    auto privileges
        = static_cast<UserPrivileges> (obj["Privileges"].get<int> ());
    user.setPrivileges(privileges);
    return user;
}

User fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToUser(obj);
}

User fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToUser(obj);
}

}

class User::UserImpl
{
public:
    std::string mName;
    std::string mPublicKey;
    std::string mHashedPassword;
    std::string mMatchingPassword;
    std::string mEmail;
    UserPrivileges mPrivileges = UserPrivileges::READ_ONLY;
    int mIdentifier = 0;
    bool mHaveName = false;
    bool mHavePublicKey = false;
    bool mHaveHashedPassword = false;
    bool mHaveEmail = false;
    bool mHaveIdentifier = false;
};

/// C'tor
User::User() :
    pImpl(std::make_unique<UserImpl> ())
{
}

/// Copy assignment
User::User(const User &user)
{
   *this = user;
}

/// Move assignment
User::User(User &&user) noexcept
{
   *this = std::move(user);
}

/// Copy assignment
User& User::operator=(const User &user)
{
    if (&user == this){return *this;}
    pImpl = std::make_unique<UserImpl> (*user.pImpl);
    return *this;
}

/// Move assignment
User& User::operator=(User &&user) noexcept
{
    if (&user == this){return *this;}
    pImpl = std::move(user.pImpl);
    return *this;
}
 
/// Destructor
User::~User() = default;

/// Reset class
void User::clear() noexcept
{
    pImpl = std::make_unique<UserImpl> ();
}

/// User name
void User::setName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("User name is empty");}
    pImpl->mName = name;
    pImpl->mHaveName = true;
}

std::string User::getName() const
{
    if (!haveName()){throw std::runtime_error("Name not set");}
    return pImpl->mName;
}

bool User::haveName() const noexcept
{
    return pImpl->mHaveName;
}

/// Email
void User::setEmail(const std::string &email)
{   
    if (isEmpty(email)){throw std::invalid_argument("Email is empty");}
    pImpl->mEmail = email;
    pImpl->mHaveEmail = true;
}

std::string User::getEmail() const
{
    if (!haveEmail()){throw std::runtime_error("Email not set");}
    return pImpl->mEmail;
}

bool User::haveEmail() const noexcept
{
    return pImpl->mHaveEmail;
}

/// Public key
void User::setPublicKey(const std::string &publicKey)
{
    if (isEmpty(publicKey)){std::invalid_argument("Public key is empty");}
    if (static_cast<int> (publicKey.length()) != getKeyLength())
    {
        throw std::invalid_argument("Public key length must be "
                                  + std::to_string(getKeyLength()));
    }
    pImpl->mPublicKey = publicKey;
    pImpl->mHavePublicKey = true;
}

std::string User::getPublicKey() const
{
    if (!havePublicKey()){throw std::runtime_error("Public key not set");}
    return pImpl->mPublicKey;
}

bool User::havePublicKey() const noexcept
{
    return pImpl->mHavePublicKey;
}

/// Password
void User::setHashedPassword(const std::string &password)
{
    if (isEmpty(password)){std::invalid_argument("Password is empty");}
    if (static_cast<int> (password.length()) > getMaximumHashedStringLength())
    {
        throw std::invalid_argument("Password length cannot exceed "
                              + std::to_string(getMaximumHashedStringLength()));
    }
    pImpl->mHashedPassword = password;
    pImpl->mMatchingPassword.clear();
    pImpl->mHaveHashedPassword = true;
}

std::string User::getHashedPassword() const
{
    if (!haveHashedPassword()){throw std::runtime_error("Password not set");}
    return pImpl->mHashedPassword;
}

bool User::haveHashedPassword() const noexcept
{
    return pImpl->mHaveHashedPassword;
}

/// Privileges
void User::setPrivileges(const UserPrivileges privileges) noexcept
{
    pImpl->mPrivileges = privileges;
}

UserPrivileges User::getPrivileges() const noexcept
{
    return pImpl->mPrivileges;
}

/// Hashed string length
int User::getMaximumHashedStringLength() const noexcept
{
    return crypto_pwhash_STRBYTES;
}

/// Text key length
int User::getKeyLength() const noexcept
{
    return 40;
}

/// Does the public key match?
bool User::doesPublicKeyMatch(const std::string &publicKey) const noexcept
{ 
    if (!havePublicKey()){return false;}
    auto keyLength = getKeyLength();
    if (static_cast<int> (publicKey.size()) != keyLength){return false;}
    return (pImpl->mPublicKey == publicKey);
}

/// Does the password match?
bool User::doesPasswordMatch(const std::string &password) const noexcept
{
    if (!haveHashedPassword()){return false;}
    // Short circuit potentially tough calculation
    if (!pImpl->mMatchingPassword.empty())
    {
        return (password == pImpl->mMatchingPassword);
    }
    auto hashedPassword = getHashedPassword();
    std::array<char, crypto_pwhash_STRBYTES> str;
    std::fill(str.begin(), str.end(), '\0');
    std::copy(hashedPassword.begin(), hashedPassword.end(), str.begin());
    auto error = crypto_pwhash_str_verify(str.data(),
                                          password.data(), password.length());
    if (error == 0)
    {
        pImpl->mMatchingPassword = password;
        return true;
    }
    return false;
}

/// Sets the user identifier number
void User::setIdentifier(const int identifier) noexcept
{
    pImpl->mIdentifier = identifier;
    pImpl->mHaveIdentifier = true;
}

int User::getIdentifier() const
{
    if (!haveIdentifier()){throw std::runtime_error("Identifier not set");}
    return pImpl->mIdentifier;
}

bool User::haveIdentifier() const noexcept
{
    return pImpl->mHaveIdentifier;
}

/// To JSON
std::string User::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// To CBOR
std::string User::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void User::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void User::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void User::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Message type
std::string User::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> User::clone() const
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> result
        = std::make_unique<User> (*this);
    return result;
}

/// Convert to message
std::string User::toMessage() const
{
    return toCBOR();
}

/// Create class from message
void User::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    User::createInstance() const noexcept
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> result
        = std::make_unique<User> (); 
    return result;
}

/// Prints the user information
std::ostream&
UMPS::Messaging::Authentication::operator<<(
    std::ostream &os, const User &user)
{
    std::string result = "User:\n";
    if (user.haveName())
    {
        result = result + "   Name: " + user.getName() + "\n";
    }
    if (user.haveEmail())
    {
        result = result + "   Email: " + user.getEmail() + "\n";
    }
    if (user.haveHashedPassword())
    {
        result = result
               + "   Hashed Password: " + user.getHashedPassword() + "\n";
    }
    if (user.havePublicKey())
    {
        result = result + "   Public Key: " + user.getPublicKey() + "\n";
    }
    if (user.haveIdentifier())
    {
        result = result + "   Identifier: "
                        + std::to_string(user.getIdentifier()) + "\n";
    }
    return os << result;
}

