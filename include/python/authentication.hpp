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
    Keys();
    Keys(const Keys &keys);
    explicit Keys(const UMPS::Authentication::Certificate::Keys &keys);
    Keys(Keys &&keys) noexcept;
    Keys& operator=(const Keys &keys);
    Keys& operator=(Keys &&keys) noexcept;
    Keys& operator=(const UMPS::Authentication::Certificate::Keys &keys);
    [[nodiscard]] UMPS::Authentication::Certificate::Keys getNativeClass() const noexcept;
    ~Keys();
    void loadFromTextFile(const std::string &fileName);
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
