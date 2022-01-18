#include "authentication/zapOptions.hpp"
#include "authentication/keys.hpp"
#include "authentication/userNameAndPassword.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "initialize.hpp"

using namespace PUMPS::Authentication;

/// C'tor
ZAPOptions::ZAPOptions() :
    pImpl(std::make_unique<UMPS::Authentication::ZAPOptions> ())
{
}

/// Copy c'tor
ZAPOptions::ZAPOptions(const ZAPOptions &options)
{
    *this = options;
}

ZAPOptions::ZAPOptions(
    const UMPS::Authentication::ZAPOptions &options)
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
    pImpl = std::make_unique<UMPS::Authentication::ZAPOptions>
            (*options.pImpl);
    return *this;
}

ZAPOptions& ZAPOptions::operator=(
    const UMPS::Authentication::ZAPOptions &options)
{
    pImpl = std::make_unique<UMPS::Authentication::ZAPOptions>
            (options);
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
UMPS::Authentication::SecurityLevel
    ZAPOptions::getSecurityLevel() const noexcept
{
    return pImpl->getSecurityLevel();
}

/// Authentication server?
bool ZAPOptions::isAuthenticationServer() const noexcept
{
    return pImpl->isAuthenticationServer();
}
/*

    void setWoodhouseClient(const UserNameAndPassword &credentials);
*/

void ZAPOptions::setStonehouseClient(const Keys &serverKeysIn,
                                     const Keys &clientKeysIn)
{
    auto serverKeys = serverKeysIn.getNativeClass();
    auto clientKeys = clientKeysIn.getNativeClass();
    pImpl->setStonehouseClient(serverKeys, clientKeys);
}

/*
    void setStonehouseServer(const Keys &serverKeys);

*/

void PUMPS::Authentication::initializeZAPOptions(pybind11::module &m)
{
    pybind11::class_<PUMPS::Authentication::ZAPOptions>
        o(m, "ZAPOptions");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This defines the ZeroMQ Authentication Protocol (ZAP) options.  Briefly, there
are four supported levels of security.

   1.  Grasslands -> There is no authentication.
   2.  Strawhouse -> The ZAP server can authenticate IP addresses.
   3.  Woodhouse  -> The ZAP server can authenticate IP addresses as well as
                     user names and password.  In this case, the Woodhouse
                     client must provide a user name and password.
   4.  Stonehouse -> The ZAP server can authenticate IP addresses as well as
                     client public keys.  In this case, the Stonehouse
                     client have the server's public key as well as its own
                     public/private key pair.

Options Properties :

   domain : The ZeroMQ domain on which authentication will be performed.

Read-only Properties :

   is_authentication_server : True indicates that this is the machine that
                              will perform authentication.
   security_level : The current security level.

)"""";

   o.def("set_grasslands_server",
         &ZAPOptions::setGrasslandsServer,
         "Sets this as a grasslands server.  This machine will accept all connections.");
   o.def("set_grasslands_client",
         &ZAPOptions::setGrasslandsClient,
         "Sets this as a grasslands client.  This machine will be able to connect to Grasslands servers.");

   o.def("set_strawhouse_server",
         &ZAPOptions::setStrawhouseServer,
         "Sets this as a strawhouse server.  This machine may validate IP addresses.");
   o.def("set_strawhouse_client",
         &ZAPOptions::setStrawhouseClient,
         "Sets this as a strawhouse client.  This machine may have its IP address validated prior to connecting to a strawhouse server.");

/*
   o.def("set_woodhouse_server",
         &ZAPOptions::setWoodhouseServer,
         "Sets this as a Woodhouse server - i.e., this machine may validate IP addresses and usernames/passwords.");
   o.def("set_woodhouse_client",
         &ZAPOptions::setWoodhouseClient,
         "Sets this as a Woodhouse client - i.e., its IP address and username/password may be validated.");
*/

   o.def("set_stonehouse_client",
         &ZAPOptions::setStonehouseClient,
         "Sets this as a Stonhouse client - i.e., its IP address is validated.  Additionally, the server's public key and this client's public and private keys must be specified");

   o.def_property("domain",
                  &ZAPOptions::getDomain,
                  &ZAPOptions::setDomain); 
   o.def_property_readonly("is_authentication_server",
                           &ZAPOptions::isAuthenticationServer);
   o.def_property_readonly("security_level",
                           &ZAPOptions::getSecurityLevel);
}
