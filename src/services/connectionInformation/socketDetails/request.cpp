#include <string>
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Request::RequestImpl
{
public:
    std::string mAddress;
    UCI::ConnectOrBind mConnectOrBind = UCI::ConnectOrBind::Bind;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::Grasslands;
};

/// C'tor
Request::Request() :
    pImpl(std::make_unique<RequestImpl> ())
{
}

/// Copy c'tor
Request::Request(const Request &socket)
{
    *this = socket;
}

/// Move c'tor
Request::Request(Request &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
Request& Request::operator=(const Request &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<RequestImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
Request& Request::operator=(Request &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
Request::~Request() = default;

/// Reset class
void Request::clear() noexcept
{
    pImpl = std::make_unique<RequestImpl> ();
}

/// Address
void Request::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string Request::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool Request::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Securtiy level
void Request::setSecurityLevel(
    const UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel Request::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Connect or bind
void Request::setConnectOrBind(
    const UCI::ConnectOrBind connectOrBind) noexcept
{
    pImpl->mConnectOrBind = connectOrBind;
}

UCI::ConnectOrBind Request::getConnectOrBind() const noexcept
{
    return pImpl->mConnectOrBind;
}

UCI::SocketType Request::getSocketType() noexcept
{
    return UCI::SocketType::Request;
}
