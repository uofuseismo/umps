#include <string>
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class XSubscriber::XSubscriberImpl
{
public:
    std::string mAddress;
    UCI::ConnectOrBind mConnectOrBind = UCI::ConnectOrBind::Connect;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::Grasslands;
};

/// C'tor
XSubscriber::XSubscriber() :
    pImpl(std::make_unique<XSubscriberImpl> ())
{
}

/// Copy c'tor
XSubscriber::XSubscriber(const XSubscriber &socket)
{
    *this = socket;
}

/// Move c'tor
XSubscriber::XSubscriber(XSubscriber &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
XSubscriber& XSubscriber::operator=(const XSubscriber &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<XSubscriberImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
XSubscriber& XSubscriber::operator=(XSubscriber &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
XSubscriber::~XSubscriber() = default;

/// Reset class
void XSubscriber::clear() noexcept
{
    pImpl = std::make_unique<XSubscriberImpl> ();
}

/// Address
void XSubscriber::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string XSubscriber::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool XSubscriber::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Securtiy level
void XSubscriber::setSecurityLevel(
    const UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel XSubscriber::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}


/// Connect or bind
void XSubscriber::setConnectOrBind(
    const UCI::ConnectOrBind connectOrBind) noexcept
{
    pImpl->mConnectOrBind = connectOrBind;
}

UCI::ConnectOrBind XSubscriber::getConnectOrBind() const noexcept
{
    return pImpl->mConnectOrBind;
}

UCI::SocketType XSubscriber::getSocketType() noexcept
{
    return UCI::SocketType::XSubscriber;
}
