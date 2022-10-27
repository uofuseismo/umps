#include "python/authentication.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "umps/authentication/zapOptions.hpp"

using namespace UMPS::Python::Authentication;
namespace UAuth = UMPS::Authentication;

///--------------------------------------------------------------------------///
///                                  Keys                                    ///
///--------------------------------------------------------------------------///
/// C'tor
Keys::Keys() :
    pImpl(std::make_unique<UAuth::Certificate::Keys> ()) 
{
}

/// Copy c'tor
Keys::Keys(const Keys &keys)
{
    *this = keys;
}

Keys::Keys(
    const UMPS::Authentication::Certificate::Keys &keys)
{
    *this = keys;
}

/// Move c'tor
Keys::Keys(Keys &&keys) noexcept
{
    *this = std::move(keys);
}

/// Copy assignment
Keys& Keys::operator=(const Keys &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<UMPS::Authentication::Certificate::Keys>
            (*options.pImpl);
    return *this;
}

Keys& Keys::operator=(
    const UMPS::Authentication::Certificate::Keys &options)
{
    pImpl = std::make_unique<UAuth::Certificate::Keys> (options);
    return *this;
}

/// Move assignment
Keys& Keys::operator=(Keys &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Native class
UAuth::Certificate::Keys Keys::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Destructor
Keys::~Keys() = default;

/// Reset class
void Keys::clear()
{
    pImpl->clear();
}

// Load the keys from a text file
void Keys::loadFromTextFile(const std::string &fileName)
{
    pImpl->loadFromTextFile(fileName);
}

///--------------------------------------------------------------------------///
///                              User Name / Password                        ///
///--------------------------------------------------------------------------///

UserNameAndPassword::UserNameAndPassword() :
    pImpl(std::make_unique<UAuth::Certificate::UserNameAndPassword> ())
{
}

UserNameAndPassword::UserNameAndPassword(
    const UserNameAndPassword &credentials)
{
    *this = credentials;
}

UserNameAndPassword::UserNameAndPassword(
    const UAuth::Certificate::UserNameAndPassword &credentials)
{
    *this = credentials;
}

UserNameAndPassword::UserNameAndPassword(
    UserNameAndPassword &&credentials) noexcept
{
    *this = std::move(credentials);
}

UserNameAndPassword&
UserNameAndPassword::operator=(const UserNameAndPassword &credentials)
{
    if (&credentials == this){return *this;}
    pImpl = std::make_unique<UAuth::Certificate::UserNameAndPassword>
            (*credentials.pImpl);
    return *this;
}

UserNameAndPassword&
UserNameAndPassword::operator=(UserNameAndPassword &&credentials) noexcept
{
    if (&credentials == this){return *this;}
    pImpl = std::move(credentials.pImpl);
    return *this;
}

UserNameAndPassword& UserNameAndPassword::operator=(
    const UAuth::Certificate::UserNameAndPassword &credentials)
{
    pImpl = std::make_unique<UAuth::Certificate::UserNameAndPassword>
            (credentials);
    return *this;
}

UserNameAndPassword::~UserNameAndPassword() = default;

UAuth::Certificate::UserNameAndPassword
    UserNameAndPassword::getNativeClass() const noexcept
{
    return *pImpl;
}

void UserNameAndPassword::setUserName(const std::string &user)
{
    pImpl->setUserName(user);
}

std::string UserNameAndPassword::getUserName() const
{
    if (!pImpl->haveUserName())
    {
        throw std::runtime_error("User name not set");
    }
    return pImpl->getUserName();
}

void UserNameAndPassword::setPassword(const std::string &password)
{
    pImpl->setPassword(password);
}

std::string UserNameAndPassword::getPassword() const
{
    if (!pImpl->havePassword())
    {
        throw std::runtime_error("Password not set");
    }
    return pImpl->getPassword();
}

void UserNameAndPassword::clear() noexcept
{
    pImpl->clear();
}
///--------------------------------------------------------------------------///
///                                 ZAP Options                              ///
///--------------------------------------------------------------------------///

/// C'tor
ZAPOptions::ZAPOptions() :
    pImpl(std::make_unique<UAuth::ZAPOptions> ()) 
{
}

/// Copy c'tor
ZAPOptions::ZAPOptions(const ZAPOptions &options)
{
    *this = options;
}

ZAPOptions::ZAPOptions(const UAuth::ZAPOptions &options)
{
    *this = options;
}

/// Move c'tor
ZAPOptions::ZAPOptions(ZAPOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ZAPOptions& ZAPOptions::operator=(const ZAPOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<UAuth::ZAPOptions> (*options.pImpl);
    return *this;
}

ZAPOptions& ZAPOptions::operator=(const UAuth::ZAPOptions &options)
{
    pImpl = std::make_unique<UAuth::ZAPOptions> (options);
    return *this;
}

/// Move assignment
ZAPOptions& ZAPOptions::operator=(ZAPOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Native class
UMPS::Authentication::ZAPOptions
    ZAPOptions::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Destructor
ZAPOptions::~ZAPOptions() = default;


/// Grasslands
void ZAPOptions::setGrasslandsClient() noexcept
{
    pImpl->setGrasslandsClient();
}

void ZAPOptions::setGrasslandsServer() noexcept
{
    pImpl->setGrasslandsServer();
}

/// Strawhouse
void ZAPOptions::setStrawhouseClient() noexcept
{
    pImpl->setStrawhouseClient();
}

void ZAPOptions::setStrawhouseServer() noexcept
{
    pImpl->setStrawhouseServer();
}

/// Woodhouse
void ZAPOptions::setWoodhouseClient(const UserNameAndPassword &credentials)
{
    auto userName = credentials.getUserName();
    auto password = credentials.getPassword();
    auto umpsCredentials = credentials.getNativeClass();
    pImpl->setWoodhouseClient(umpsCredentials);
}

void ZAPOptions::setWoodhouseServer() noexcept
{
    pImpl->setWoodhouseServer();
}

/// Domain
void ZAPOptions::setDomain(const std::string &domain)
{
    pImpl->setDomain(domain);
}

std::string ZAPOptions::getDomain() const noexcept
{
    return pImpl->getDomain();
}

/// Security level
UAuth::SecurityLevel ZAPOptions::getSecurityLevel() const noexcept
{
    return pImpl->getSecurityLevel();
}

/// Authentication server?
bool ZAPOptions::isAuthenticationServer() const noexcept
{
    return pImpl->isAuthenticationServer();
}

/// Stonehouse
void ZAPOptions::setStonehouseClient(const Keys &serverKeysIn,
                                     const Keys &clientKeysIn)
{
    auto serverKeys = serverKeysIn.getNativeClass();
    auto clientKeys = clientKeysIn.getNativeClass();
    pImpl->setStonehouseClient(serverKeys, clientKeys);
}

void ZAPOptions::setStonehouseServer(const Keys &serverKeysIn)
{
    auto serverKeys = serverKeysIn.getNativeClass();
    pImpl->setStonehouseServer(serverKeys);
}

///--------------------------------------------------------------------------///
///                             Initialize Modules                           ///
///--------------------------------------------------------------------------///
/// Initialize
void UMPS::Python::Authentication::initialize(pybind11::module &m)
{
    pybind11::module auth = m.def_submodule("Authentication");
    auth.attr("__doc__") = "Utilities for connection authentication in UMPS.";

    pybind11::enum_<UMPS::Authentication::SecurityLevel> (auth, "SecurityLevel")
        .value("Grasslands", UMPS::Authentication::SecurityLevel::Grasslands,
               "All authentications are accepted.")
        .value("Strawhouse", UMPS::Authentication::SecurityLevel::Strawhouse,
               "IP addresses may be validated.")
        .value("Woodhouse",  UMPS::Authentication::SecurityLevel::Woodhouse,
               "A user name and password must be provided and IP addresses may be validated.")
        .value("Stonehouse",  UMPS::Authentication::SecurityLevel::Stonehouse,
               "A key exchange will be performed and IP addresses may be validated.");

    //-----------------------------------Key Pair-----------------------------//
    pybind11::class_<Keys> keys(auth, "Keys");
    keys.def(pybind11::init<> ());
    keys.doc() = R""""(
This defines the public/private keypair used by the ZeroMQ Authentication Protocol (ZAP).
Typically, a Stonehouse client will be used from the Python API.  In this case, the
client's public and private keys must be known as well as the server's public key.
)"""";
    keys.def("__copy__", [](const Keys &self)
    {   
        return Keys(self);
    }); 
    keys.def("load",
             &Keys::loadFromTextFile,
             "Loads the credentials from a text file.");
    keys.def("clear",
             &Keys::clear,
             "Clears all stored credentials and releases memory.");
    //---------------------------User Name and Password ----------------------//
    pybind11::class_<UserNameAndPassword> up(auth, "UserNameAndPassword");
    up.def(pybind11::init<> ());
    up.doc() = R""""(
This defines a user name and password pair.  This form of authentication is
slow.  Additionally, the passwords are not encrypted prior to transmission.
It is recommended to use the Stonehouse security model.

Properties :

   user_name : str
       The user name.  This is only necessary for a client.
   password : str
       The corresponding password.  This is only necessary for a client.

Read-only Properties :

   is_authentication_server : bool
       True indicates that this is the machine that will perform authentication.
   security_level : int
       The current security level.

)"""";
    up.def("__copy__", [](const UserNameAndPassword &self)
    {
        return UserNameAndPassword(self);
    });
    up.def_property("user_name",
                    &UserNameAndPassword::getUserName,
                    &UserNameAndPassword::setUserName);
    up.def_property("password",
                    &UserNameAndPassword::getPassword,
                    &UserNameAndPassword::setPassword);
    up.def("clear",
           &UserNameAndPassword::getPassword,
           "Resets the username and password");

    //---------------------------------ZAP Options----------------------------//
    pybind11::class_<ZAPOptions> zap(auth, "ZAPOptions");
    zap.def(pybind11::init<> ());
    zap.doc() = R""""(
This defines the ZeroMQ Authentication Protocol (ZAP) options.  Briefly, there
are four supported levels of security.

   1.  Grasslands -> There is no authentication.
   2.  Strawhouse -> The ZAP server can authenticate IP addresses.
   3.  Woodhouse  -> The ZAP server can authenticate IP addresses as well as
                     a user name and password pair.  In this case, the Woodhouse
                     client must provide a user name and password.
   4.  Stonehouse -> The ZAP server can authenticate IP addresses as well as
                     a client's public keys.  In this case, the Stonehouse
                     client must have the server's public key as well as its own
                     public/private key pair.

Read-only Properties :

   is_authentication_server : bool
      True indicates that this is the machine that will perform authentication.
   security_level : int
      The current security level.

Optional Properties :

   domain : The ZeroMQ domain on which authentication will be performed.
            By default this is global.

)"""";
    zap.def("__copy__", [](const ZAPOptions &self)
    {
        return ZAPOptions(self);
    }); 
    zap.def("set_grasslands_server",
            &ZAPOptions::setGrasslandsServer,
            "Sets this as a grasslands server.  This machine will accept all connections.");
    zap.def("set_grasslands_client",
            &ZAPOptions::setGrasslandsClient,
            "Sets this as a grasslands client.  This machine will be able to connect to Grasslands servers.");

    zap.def("set_strawhouse_server",
            &ZAPOptions::setStrawhouseServer,
            "Sets this as a strawhouse server.  This machine may validate IP addresses.");
    zap.def("set_strawhouse_client",
            &ZAPOptions::setStrawhouseClient,
            "Sets this as a strawhouse client.  This machine may have its IP address validated prior to connecting to a strawhouse server.");

    zap.def("set_woodhouse_server",
            &ZAPOptions::setWoodhouseServer,
            "Sets this as a Woodhouse server - i.e., this machine may validate IP addresses and usernames/passwords.");
    zap.def("set_woodhouse_client",
            &ZAPOptions::setWoodhouseClient,
            "Sets this as a Woodhouse client - i.e., its IP address and username/password may be validated.");

    zap.def("set_stonehouse_client",
            &ZAPOptions::setStonehouseClient,
            "Sets this as a Stonhouse client - i.e., its IP address is validated.  Additionally, the server's public key and this client's public and private keys must be specified");

    zap.def_property("domain",
                     &ZAPOptions::getDomain,
                     &ZAPOptions::setDomain);
    zap.def_property_readonly("is_authentication_server",
                              &ZAPOptions::isAuthenticationServer);
    zap.def_property_readonly("security_level",
                              &ZAPOptions::getSecurityLevel);
}
