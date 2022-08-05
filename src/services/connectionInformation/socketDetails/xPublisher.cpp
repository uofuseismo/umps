#include <string>
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class XPublisher::XPublisherImpl
{
public:
    std::string mAddress;
    UCI::ConnectOrBind mConnectOrBind = UCI::ConnectOrBind::Bind;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
};

/// C'tor
XPublisher::XPublisher() :
    pImpl(std::make_unique<XPublisherImpl> ())
{
}

/// Copy c'tor
XPublisher::XPublisher(const XPublisher &socket)
{
    *this = socket;
}

/// Move c'tor
XPublisher::XPublisher(XPublisher &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
XPublisher& XPublisher::operator=(const XPublisher &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<XPublisherImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
XPublisher& XPublisher::operator=(XPublisher &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
XPublisher::~XPublisher() = default;

/// Reset class
void XPublisher::clear() noexcept
{
    pImpl = std::make_unique<XPublisherImpl> ();
}

/// Address
void XPublisher::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string XPublisher::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool XPublisher::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Securtiy level
void XPublisher::setSecurityLevel(
    const UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel XPublisher::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

void XPublisher::setConnectOrBind(
    const UCI::ConnectOrBind connectOrBind) noexcept
{
    pImpl->mConnectOrBind = connectOrBind;
}

UCI::ConnectOrBind XPublisher::getConnectOrBind() const noexcept
{
    return pImpl->mConnectOrBind;
}

UCI::SocketType XPublisher::getSocketType() noexcept
{
    return UCI::SocketType::XPublisher;
}
