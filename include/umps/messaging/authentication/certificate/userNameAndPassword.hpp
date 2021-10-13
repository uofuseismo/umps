#ifndef UMPS_MESSAGING_AUTHENTICATION_CERTIFICATE_USERNAMEANDPASSWORD_HPP
#define UMPS_MESSAGING_AUTHENTICATION_CERTIFICATE_USERNAMEANDPASSWORD_HPP
#include <memory>
#include "umps/messaging/authentication/certificate/enums.hpp"
namespace UMPS::Messaging::Authentication::Certificate
{
/// @class UserNameAndPassword "userNameAndPassword.hpp" "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
/// @brief This is a certificate based on a username and password.  
/// @note This form of authentication in UMPS is not encrypted prior to 
///       transmission so if an unauthorized person is monitoring network
///       traffic then these credentials can be stolen. 
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class UserNameAndPassword
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    UserNameAndPassword();
    /// @brief Copy constructor.
    /// @param[in] plainText   The certificate from which to initialize
    ///                        this class.
    UserNameAndPassword(const UserNameAndPassword &plainText);
    /// @brief Move constructor.
    /// @param[in,out] plainText  The certificate from which to initialize
    ///                           this class.  On exit, certificate's behavior
    ///                           is undefined.
    UserNameAndPassword(UserNameAndPassword &&plainText) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @result A deep copy of the plainText credentials.
    /// @param[in] plainText  The certificate to copy to this.
    /// @result A deep copy of plainText.
    UserNameAndPassword& operator=(const UserNameAndPassword &plainText);
    /// @brief Move assignment operator.
    /// @param[in,out] plainText  The certificate whose memory will be moved to
    ///                           this.  On exit, plainText's behavior is
    ///                           undefined.
    /// @result The memory from plainText moved to this.
    UserNameAndPassword& operator=(UserNameAndPassword &&plainText) noexcept;
    /// @}

    /// @name User Name (Required)
    /// @{
    /// @brief Sets the user name.
    /// @param[in] userName  The user name.
    void setUserName(const std::string &userName) noexcept;
    /// @result The user name.
    /// @throws std::runtime_error if \c haveUserName() is false.
    [[nodiscard]] std::string getUserName() const;
    /// @result True indicates the user name was set.
    [[nodiscard]] bool haveUserName() const noexcept;
    /// @}

    /// @name Password (Required)
    /// @{
    /// @brief Sets the password.
    /// @param[in] password   The password.
    void setPassword(const std::string &password) noexcept;
    /// @result The password.
    /// @throws std::runtime_error if \c havePassword() is false.
    [[nodiscard]] std::string getPassword() const;
    /// @result True indicates the password was set.
    [[nodiscard]] bool havePassword() const noexcept;

    /// @param[in] level  Controls the strength of the hashing technique
    ///                   when creating a hashed variant of the password.
    /// @result The hashed variant of the password.
    /// @throws std::runtime_error if \c havePassword() is false.
    [[nodiscard]] std::string getHashedPassword(HashLevel level = HashLevel::SENSITIVE) const;
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Releases all memory and resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~UserNameAndPassword(); 
    /// @}
private:
    class UserNameAndPasswordImpl;
    std::unique_ptr<UserNameAndPasswordImpl> pImpl;
};
}
#endif
