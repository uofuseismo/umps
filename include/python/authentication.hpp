#ifndef UMPS_PYTHON_AUTHENTICATION_HPP
#define UMPS_PYTHON_AUTHENTICATION_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include <umps/authentication/zapOptions.hpp>
#include <umps/authentication/certificate/keys.hpp>
#include <umps/authentication/certificate/userNameAndPassword.hpp>
namespace UMPS::Python::Authentication
{
/// @brief Defines a username and password certificate.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class UserNameAndPassword
{
public:
    UserNameAndPassword();
    UserNameAndPassword(const UserNameAndPassword &credentials);
    explicit UserNameAndPassword(const UMPS::Authentication::Certificate::UserNameAndPassword &credentials);
    UserNameAndPassword(UserNameAndPassword &&credentials) noexcept;
    UserNameAndPassword& operator=(const UserNameAndPassword &credentials);
    UserNameAndPassword& operator=(UserNameAndPassword &&credentials) noexcept;
    UserNameAndPassword& operator=(const UMPS::Authentication::Certificate::UserNameAndPassword &credentials);
    [[nodiscard]] const UMPS::Authentication::Certificate::UserNameAndPassword& getNativeClassReference() const noexcept;
    [[nodiscard]] UMPS::Authentication::Certificate::UserNameAndPassword getNativeClass() const noexcept;
    void setPassword(const std::string &password);
    [[nodiscard]] std::string getPassword() const;
    void setUserName(const std::string &user);
    [[nodiscard]] std::string getUserName() const;
    void clear() noexcept;
    ~UserNameAndPassword();

private:
    std::unique_ptr<UMPS::Authentication::Certificate::UserNameAndPassword> pImpl;
};
/// @brief Defines a public/private keypair certificate.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Keys
{
public:
    /// @brief Constructor.
    Keys();
    /// @brief Copy constructor.
    Keys(const Keys &keys);
    /// @brief Constructs this class from the native UMPS class.
    explicit Keys(const UMPS::Authentication::Certificate::Keys &keys);
    /// @brief Move constructor.
    Keys(Keys &&keys) noexcept;
    /// @brief Copy assignment.
    Keys& operator=(const Keys &keys);
    /// @brief Move assignment.
    Keys& operator=(Keys &&keys) noexcept;
    /// @brief Make a copy of this class from the native UMPS class.
    Keys& operator=(const UMPS::Authentication::Certificate::Keys &keys);
    /// @result A reference to the native class.
    [[nodiscard]] const UMPS::Authentication::Certificate::Keys& getNativeClassReference() const noexcept;
    /// @result A copy of the native class.
    [[nodiscard]] UMPS::Authentication::Certificate::Keys getNativeClass() const noexcept;
    /// @brief Destructor.
    ~Keys();
    /// @brief Loads the key from a text file.
    void loadFromTextFile(const std::string &fileName);
    /// @brief Sets the private key.
    void setPrivateKey(const std::string &privateKey);
    /// @result The private key.
    [[nodiscard]] std::string getPrivateKey() const;
    /// @brief Sets the public key.
    void setPublicKey(const std::string &publicKey);
    /// @result The public key.
    [[nodiscard]] std::string getPublicKey() const;
    /// @brief Sets metadata for the key.
    void setMetadata(const std::string &metadata);
    /// @result The keys metadata.
    [[nodiscard]] std::string getMetadata() const; 
    /// @brief Writes the public key to a text file.
    void writePublicKeyToTextFile(const std::string &fileName) const;
    /// @brief Writes the private key to a text file.
    void writePrivateKeyToTextFile(const std::string &fileName) const;
    /// @brief Resets the class.
    void clear();
private:
    std::unique_ptr<UMPS::Authentication::Certificate::Keys> pImpl;
};
/// @brief Defines ZeroMQ authentication protocol options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class ZAPOptions
{
public:

    ZAPOptions();
    ZAPOptions(const ZAPOptions &options);
    explicit ZAPOptions(const UMPS::Authentication::ZAPOptions &options);
    ZAPOptions(ZAPOptions &&options) noexcept;
    ZAPOptions& operator=(const ZAPOptions &options);
    ZAPOptions& operator=(ZAPOptions &&options) noexcept;
    ZAPOptions& operator=(const UMPS::Authentication::ZAPOptions &options);
    [[nodiscard]] const UMPS::Authentication::ZAPOptions& getNativeClassReference() const noexcept;
    [[nodiscard]] UMPS::Authentication::ZAPOptions getNativeClass() const noexcept;
    ~ZAPOptions();

    void setGrasslandsClient() noexcept;
    void setGrasslandsServer() noexcept;

    void setStrawhouseClient() noexcept;
    void setStrawhouseServer() noexcept;

    void setWoodhouseClient(const UserNameAndPassword &credentials);
    void setWoodhouseServer() noexcept;

    void setStonehouseClient(const Keys &serverKeys,
                             const Keys &clientKeys);
    void setStonehouseServer(const Keys &serverKeys);

    void setDomain(const std::string &domain);
    [[nodiscard]] std::string getDomain() const noexcept;

    [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept;

    [[nodiscard]] bool isAuthenticationServer() const noexcept;
private:
    std::unique_ptr<UMPS::Authentication::ZAPOptions> pImpl; 
};
void initialize(pybind11::module &m);
}
#endif
