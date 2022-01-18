#include "authentication/keys.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "initialize.hpp"

using namespace PUMPS::Authentication;

/// C'tor
Keys::Keys() :
    pImpl(std::make_unique<UMPS::Authentication::Certificate::Keys> ()) 
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
    pImpl = std::make_unique<UMPS::Authentication::Certificate::Keys>
            (options);
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
UMPS::Authentication::Certificate::Keys
    Keys::getNativeClass() const noexcept
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

/// Initialize
void PUMPS::Authentication::initializeKeys(pybind11::module &m) 
{
    pybind11::class_<PUMPS::Authentication::Keys> o(m, "Keys");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This defines the public/private keypair used by the ZeroMQ Authentication Protocol (ZAP).
Typically, a Stonehouse client will be used from the Python API.  In this case, the
client's public and private keys must be known as well as the server's public key.

Options Properties :

   domain : The ZeroMQ domain on which authentication will be performed.

Read-only Properties :

   is_authentication_server : True indicates that this is the machine that
                              will perform authentication.
   security_level : The current security level.

)"""";

   o.def("load",
         &Keys::loadFromTextFile,
         "Loads the credentials from a text file.");
   o.def("clear",
         &Keys::clear,
         "Clears all stored credentials and releases memory.");
}

