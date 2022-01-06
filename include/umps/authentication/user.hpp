#ifndef UMPS_AUTHENTICATION_USER_HPP
#define UMPS_AUTHENTICATION_USER_HPP
#include <memory>
#include <array>
#include <string>
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/enums.hpp"
namespace UMPS::Authentication::Certificate
{
 class Keys;
 class UserNameAndPassword;
}
namespace UMPS::Authentication
{
/// @class User "user.hpp" "umps/authentication/user.hpp"
/// @brief This class defines a user (row) in the user authentication table.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class User : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    User();
    /// @brief Copy constructor.
    /// @param[in] user  The user class from which to initialize this class.
    User(const User &user);
    /// @brief Move constructor.
    /// @param[in,out] user  The user class from which to initialize this class.
    ///                      On exit, user's behavior is undefined.
    User(User &&user) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @param[in] user  The user class to copy to this.
    /// @result A deep copy of the user class.
    User& operator=(const User &user);
    /// @result The memory from user moved to this.
    User& operator=(User &&user) noexcept;
    /// @}

    /// @name Name 
    /// @{
    /// @brief Sets the user's name.
    /// @throws std::invalid_argument if the name is empty.
    void setName(const std::string &name);
    /// @result The user's name.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates that the user's name was set.
    [[nodiscard]] bool haveName() const noexcept;
    /// @}

    /// @name Email
    /// @{
    /// @brief Sets the user's email address.
    /// @param[in] email   The email address.
    /// @throws std::invalid_argument if the email address is NULL.
    void setEmail(const std::string &email);
    /// @result The email address.
    /// @throws std::runtime_Error if \c haveEmail() is false.
    [[nodiscard]] std::string getEmail() const;
    /// @result True indicates that the email address was set.
    [[nodiscard]] bool haveEmail() const noexcept;
    /// @}

    /// @name Password
    /// @{
    /// @brief Sets the hashed password.
    /// @param[in] password  The hashed password.  This must have length
    ///                      at most \c getMaximumHashedStringLength().
    /// @throws std::invalid_argument if the password is too long.
    void setHashedPassword(const std::string &password);
    /// @result The hashed password.
    /// @throws std::runtime_error if \c haveHashedPassword() is false.
    [[nodiscard]] std::string getHashedPassword() const;
    /// @result True indicates that the hashed password was set.
    [[nodiscard]] bool haveHashedPassword() const noexcept;
 
    /// @brief Determines if the password matches the previously specified
    ///        hashed password.
    /// @param[in] password  The password to compare with the hashed password.
    /// @note This returns false is \c haveHashedPassword() is false.
    [[nodiscard]] bool doesPasswordMatch(const std::string &password) const noexcept;
    /// @}

    /// @name Public Key
    /// @{
    /// @brief Sets the public key.
    /// @param[in] publicKey  The user's hashed public key.  This must have
    ///                       length at most \c getKeyLength().
    /// @throws std::invalid_argument if the public key has the wrong length.
    void setPublicKey(const std::string &publicKey);
    /// @result The public key.
    /// @throws std::runtime_error if \c havePublicKey() is false.
    [[nodiscard]] std::string getPublicKey() const; 
    /// @result True indicates that the public key was set.
    [[nodiscard]] bool havePublicKey() const noexcept;

    /// @brief Determines if the text-based public key matches the previously
    ///        specified hashed public key.
    /// @param[in] publicKey  The public key to compare with the given
    ///                       public key.
    /// @result True indicates that the public key matches the hashed
    ///         public key.
    /// @note This will return false is \c havePublicKey() is false.
    [[nodiscard]] bool doesPublicKeyMatch(const std::string &publicKey) const noexcept;
    /// @}

    /// @name Privileges
    /// @{
    /// @brief Sets the user's privileges.
    /// @param[in] privileges  The user's privileges.
    void setPrivileges(const UserPrivileges privileges) noexcept;
    /// @result The user's privileges.  By default this is read-only.
    [[nodiscard]] UserPrivileges getPrivileges() const noexcept;
    /// @}

    /// @name Identifier Number
    /// @{
    /// @brief Sets the user's unique identifier number.  
    /// @param[in] identifier  The identifier number.
    void setIdentifier(int identifier) noexcept; 
    /// @result The user's identifier number.
    /// @throws std::runtime_error if \c haveIdentifier() is false.
    [[nodiscard]] int getIdentifier() const;
    /// @result True indicates the user's identifier number has been set.
    [[nodiscard]] bool haveIdentifier() const noexcept;
    /// @}

    /// @result The length of a hashed string.
    [[nodiscard]] int getMaximumHashedStringLength() const noexcept;
    /// @result The length of the public key.
    [[nodiscard]] int getKeyLength() const noexcept;

    /// @name Destructors
    /// @{
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~User();
    /// @}

    /// @name Message Properties
    /// @{
    /// @brief Creates a copy of this class.
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<IMessage> clone() const override final;
    /// @brief Creates an instance of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept final;
    /// @brief Serializes this class into a message.
    [[nodiscard]] std::string toMessage() const final;
    /// @brief Converts this message from a string representation to data.
    void fromMessage(const char *data, const size_t length) override final;
    /// @result The message type.
    [[nodiscard]] std::string getMessageType() const noexcept override final;
    /// @}

    /// @name Debugging Properties
    /// @{
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables this.
    /// @result This class converted to a JSON representation.
    [[nodiscard]] std::string toJSON(const int nIndent =-1) const;
    /// @result This class converted to  CBOR representation.
    [[nodiscard]] std::string toCBOR() const;
    /// @brief Creates the class from a CBOR message.
    /// @param[in] cbor  The CBOR message.
    void fromCBOR(const std::string &cbor);
    /// @brief Creates the class from a JSON message.
    /// @param[in] message  The message from which to create this class.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Creates the class from a CBOR message.
    /// @param[in] data    The contents of the CBOR message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0.
    void fromCBOR(const uint8_t *data, size_t length);
    /// @}
private:
    class UserImpl;
    std::unique_ptr<UserImpl> pImpl;    
};
/// Prints the user information to stdout for debugging
std::ostream& operator<<(std::ostream &os, const User &user);
}
#endif
