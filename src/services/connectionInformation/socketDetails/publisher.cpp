#include <string>
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Publisher::PublisherImpl
{
public:
    std::string mAddress;
    UCI::ConnectOrBind mConnectOrBind = UCI::ConnectOrBind::BIND;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> ())
{
}

/// Copy c'tor
Publisher::Publisher(const Publisher &socket)
{
    *this = socket;
}

/// Move c'tor
Publisher::Publisher(Publisher &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
Publisher& Publisher::operator=(const Publisher &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<PublisherImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
Publisher& Publisher::operator=(Publisher &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
Publisher::~Publisher() = default;

/// Reset class
void Publisher::clear() noexcept
{
    pImpl = std::make_unique<PublisherImpl> ();
}

/// Address
void Publisher::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string Publisher::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool Publisher::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Securtiy level
void Publisher::setSecurityLevel(
    const UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel Publisher::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Connect or bind
void Publisher::setConnectOrBind(
    const UCI::ConnectOrBind connectOrBind) noexcept
{
    pImpl->mConnectOrBind = connectOrBind;
}

UCI::ConnectOrBind Publisher::getConnectOrBind() const noexcept
{
    return pImpl->mConnectOrBind;
}

UCI::SocketType Publisher::getSocketType() noexcept
{
    return UCI::SocketType::PUBLISHER;
}
