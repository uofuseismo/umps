#ifndef PRIVATE_AUTHENTICATION_ZAPOPTIONS_HPP
#define PRIVATE_AUTHENTICATION_ZAPOPTIONS_HPP
#include <zmq.hpp>
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "private/isEmpty.hpp"
namespace
{

void setStrawhouse(zmq::socket_t *socket,
                   const bool isAuthenticationServer,
                   const std::string zapDomain)
{
    if (isAuthenticationServer)
    {   
        if (isEmpty(zapDomain))
        {
            throw std::invalid_argument("ZAP domain is empty");
        }
        socket->set(zmq::sockopt::zap_domain, zapDomain); 
    }
}

void setWoodhouse(
    zmq::socket_t *socket,
    const UMPS::Messaging::Authentication::Certificate::UserNameAndPassword
         &credentials,
    const bool isAuthenticationServer,
    const std::string &zapDomain)
{
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!isAuthenticationServer)
    {
        if (!credentials.haveUserName())
        {
            throw std::invalid_argument("Username must be set for ZAP client");
        }
        if (!credentials.havePassword())
        {
            throw std::invalid_argument("Password must be set for ZAP client");
        }
    }
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

void setStonehouseServer(
    zmq::socket_t *socket,
    const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
    const std::string &zapDomain)
{
    if (isEmpty(zapDomain)){throw std::invalid_argument("ZAP domain is empty");}
    if (!serverKeys.havePublicKey())
    {
        throw std::invalid_argument("Server public key not set");
    }
    if (!serverKeys.havePrivateKey())
    {
        throw std::invalid_argument("Server private key not set");
    }
    auto serverKey = serverKeys.getPublicTextKey();
    auto privateKey = serverKeys.getPrivateTextKey();
    socket->set(zmq::sockopt::zap_domain, zapDomain);
    socket->set(zmq::sockopt::curve_server, 1); 
    socket->set(zmq::sockopt::curve_publickey, serverKey.data());
    socket->set(zmq::sockopt::curve_secretkey, privateKey.data()); 
}

void setStonehouseClient(
    zmq::socket_t *socket,
    const UMPS::Messaging::Authentication::Certificate::Keys &serverKeys,
    const UMPS::Messaging::Authentication::Certificate::Keys &clientKeys,
    const std::string &zapDomain)
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

}
#endif
