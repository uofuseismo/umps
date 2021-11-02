#include <string>
#include <cassert>
#include <zmq.hpp>
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::Authentication;

namespace
{

void setStrawhouseSocketOptions(zmq::socket_t *socket,
                                const bool isAuthenticationServer,
                                const std::string zapDomain)
{
    if (isAuthenticationServer)
    {
        socket->set(zmq::sockopt::zap_domain, zapDomain);
    }
}

void setWoodhouseSocketOptions(
    zmq::socket_t *socket,
    const Certificate::UserNameAndPassword &credentials,
    const bool isAuthenticationServer,
    const std::string &zapDomain)
{
    socket->set(zmq::sockopt::zap_domain, zapDomain);
    if (!isAuthenticationServer)
    {
        socket->set(zmq::sockopt::plain_server, 0);
        socket->set(zmq::sockopt::plain_username,
                    credentials.getUserName());
        socket->set(zmq::sockopt::plain_password,
                    credentials.getPassword());
    }
    else
    {
        socket->set(zmq::sockopt::plain_server, 1);
    }
}

void setStonehouseClientSocketOptions(zmq::socket_t *socket,
                                      const Certificate::Keys &serverKeys,
                                      const Certificate::Keys &clientKeys,
                                      const std::string &zapDomain)
{
    auto serverPublicKey  = serverKeys.getPublicTextKey();
    auto clientPublicKey  = clientKeys.getPublicTextKey();
    auto clientPrivateKey = clientKeys.getPrivateTextKey();

    socket->set(zmq::sockopt::zap_domain, zapDomain);
    socket->set(zmq::sockopt::curve_server, 0); 
    socket->set(zmq::sockopt::curve_serverkey,
                serverPublicKey.data());
    socket->set(zmq::sockopt::curve_publickey,
                clientPublicKey.data());
    socket->set(zmq::sockopt::curve_secretkey,
                clientPrivateKey.data());
}

void setStonehouseServerSocketOptions(zmq::socket_t *socket,
                                      const Certificate::Keys &serverKeys,
                                      const std::string &zapDomain)
{
    auto serverKey = serverKeys.getPublicTextKey();
    auto privateKey = serverKeys.getPrivateTextKey();
    socket->set(zmq::sockopt::zap_domain, zapDomain);
    socket->set(zmq::sockopt::curve_server, 1);
    socket->set(zmq::sockopt::curve_publickey, serverKey.data());
    socket->set(zmq::sockopt::curve_secretkey, privateKey.data());
}

}

///--------------------------------------------------------------------------///
///                              Implementation                              ///
///--------------------------------------------------------------------------///

class ZAPOptions::ZAPOptionsImpl
{
public:
    Certificate::UserNameAndPassword mClientCredentials;
    Certificate::Keys mServerKeys;
    Certificate::Keys mClientKeys;
    std::string mDomain = "global";
    SecurityLevel mSecurityLevel = SecurityLevel::GRASSLANDS;
    bool mIsAuthenticationServer = false;
};

/// C'tor
ZAPOptions::ZAPOptions() :
    pImpl(std::make_unique<ZAPOptionsImpl> ())
{
}

/// Copy c'tor
ZAPOptions::ZAPOptions(const ZAPOptions &options)
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
    pImpl = std::make_unique<ZAPOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ZAPOptions& ZAPOptions::operator=(ZAPOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
ZAPOptions::~ZAPOptions() = default;

/// Reset class
void ZAPOptions::clear() noexcept
{
    pImpl = std::make_unique<ZAPOptionsImpl> ();
}

/// Grasslands
void ZAPOptions::setGrasslandsServer() noexcept
{
    pImpl->mSecurityLevel = SecurityLevel::GRASSLANDS;
    pImpl->mIsAuthenticationServer = true;
}

void ZAPOptions::setGrasslandsClient() noexcept
{
    pImpl->mSecurityLevel = SecurityLevel::GRASSLANDS;
    pImpl->mIsAuthenticationServer = false;
}

/// Strawhouse
void ZAPOptions::setStrawhouseServer() noexcept
{
    pImpl->mSecurityLevel = SecurityLevel::STRAWHOUSE;
    pImpl->mIsAuthenticationServer = true;
}

void ZAPOptions::setStrawhouseClient() noexcept
{
    pImpl->mSecurityLevel = SecurityLevel::STRAWHOUSE;
    pImpl->mIsAuthenticationServer = false;
}

/// Woodhouse
void ZAPOptions::setWoodhouseServer() noexcept
{
    pImpl->mSecurityLevel = SecurityLevel::WOODHOUSE;
    pImpl->mIsAuthenticationServer = true;
}

void ZAPOptions::setWoodhouseClient(
    const Certificate::UserNameAndPassword &credentials)
{
    if (!credentials.haveUserName())
    {
        throw std::invalid_argument("Username must be set for ZAP client");
    }
    if (!credentials.havePassword())
    {
        throw std::invalid_argument("Password must be set for ZAP client");
    }
    pImpl->mClientCredentials = credentials;
    pImpl->mSecurityLevel = SecurityLevel::WOODHOUSE;
    pImpl->mIsAuthenticationServer = false;
}

/// Stonehouse 
void ZAPOptions::setStonehouseServer(const Certificate::Keys &serverKeys)
{
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!serverKeys.havePrivateKey())
    {
        throw std::invalid_argument("Server private key not set");
    }
    pImpl->mServerKeys = serverKeys;
    pImpl->mSecurityLevel = SecurityLevel::STONEHOUSE;
    pImpl->mIsAuthenticationServer = true;
}

void ZAPOptions::setStonehouseClient(const Certificate::Keys &serverKeys,
                                     const Certificate::Keys &clientKeys)
{
    if (!serverKeys.havePublicKey())
    {   
        throw std::invalid_argument("Server public key not set");
    }
    if (!clientKeys.havePublicKey())
    {
        throw std::invalid_argument("Client public key not set");
    }
    if (!clientKeys.havePrivateKey())
    {
        throw std::invalid_argument("Client private key not set");
    }
    pImpl->mServerKeys = serverKeys;
    pImpl->mClientKeys = clientKeys;
    pImpl->mSecurityLevel = SecurityLevel::STONEHOUSE;
    pImpl->mIsAuthenticationServer = false;
}

/// Domain
void ZAPOptions::setDomain(const std::string &domain)
{
    if (isEmpty(domain)){throw std::invalid_argument("Domain is empty");}
    pImpl->mDomain = domain;
}

std::string ZAPOptions::getDomain() const noexcept
{
    return pImpl->mDomain;
}

/// Client credentials
Certificate::UserNameAndPassword ZAPOptions::getClientCredentials() const
{
    if (getSecurityLevel() != SecurityLevel::WOODHOUSE)
    {
        throw std::runtime_error("Security level must be woodhouse");
    }
    if (isAuthenticationServer())
    {
        throw std::runtime_error("Credentials only set for client");
    }
    return pImpl->mClientCredentials; 
}

/// Client keys
Certificate::Keys ZAPOptions::getServerKeys() const
{
    if (getSecurityLevel() != SecurityLevel::STONEHOUSE)
    {
        throw std::runtime_error("Security level must be stonehouse");
    }
    return pImpl->mServerKeys;
}

Certificate::Keys ZAPOptions::getClientKeys() const
{
    if (getSecurityLevel() != SecurityLevel::STONEHOUSE)
    {
        throw std::runtime_error("Security level must be stonehouse");
    }
    if (isAuthenticationServer())
    {
        throw std::runtime_error("Client keys not set for server");
    }
    return pImpl->mClientKeys;
}

/// Authentication server?
bool ZAPOptions::isAuthenticationServer() const noexcept
{
    return pImpl->mIsAuthenticationServer;
}

/// Security level
SecurityLevel ZAPOptions::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Set options on socket
void ZAPOptions::setSocketOptions(zmq::socket_t *socket) const
{
    // Nothing to do
    auto securityLevel = getSecurityLevel();
    if (securityLevel == SecurityLevel::GRASSLANDS){return;}
    // Verify socket isn't null
    if (socket == nullptr){throw std::invalid_argument("Socket is NULL");}
    // Get common information
    auto isAuthServer = isAuthenticationServer();
    auto zapDomain = getDomain();
    if (securityLevel == SecurityLevel::STRAWHOUSE)
    {
        setStrawhouseSocketOptions(socket, isAuthServer, zapDomain);
    }
    else if (securityLevel == SecurityLevel::WOODHOUSE)
    {
        Certificate::UserNameAndPassword credentials;
        if (!isAuthServer){credentials = getClientCredentials();}
        setWoodhouseSocketOptions(socket, credentials, isAuthServer, zapDomain);
    }
    else if (securityLevel == SecurityLevel::STONEHOUSE)
    {
        auto serverKeys = getServerKeys();
        if (isAuthServer)
        {
            setStonehouseServerSocketOptions(socket, serverKeys, zapDomain);
        }
        else
        {
            auto clientKeys = getClientKeys();
            setStonehouseClientSocketOptions(socket, serverKeys,
                                             clientKeys, zapDomain);
        }
    }
    else
    {
        assert(false);
        throw std::runtime_error("Security level not handled");
    }
}
