#include <string>
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Subscriber::SubscriberImpl
{
public:
    std::string mAddress;
    UCI::ConnectOrBind mConnectOrBind = UCI::ConnectOrBind::Bind;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
};

/// C'tor
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> ())
{
}

/// Copy c'tor
Subscriber::Subscriber(const Subscriber &socket)
{
    *this = socket;
}

/// Move c'tor
Subscriber::Subscriber(Subscriber &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
Subscriber& Subscriber::operator=(const Subscriber &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<SubscriberImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
Subscriber& Subscriber::operator=(Subscriber &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
Subscriber::~Subscriber() = default;

/// Reset class
void Subscriber::clear() noexcept
{
    pImpl = std::make_unique<SubscriberImpl> ();
}

/// Address
void Subscriber::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string Subscriber::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool Subscriber::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Securtiy level
void Subscriber::setSecurityLevel(
    const UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel Subscriber::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Connect or bind
void Subscriber::setConnectOrBind(
    const UCI::ConnectOrBind connectOrBind) noexcept
{
    pImpl->mConnectOrBind = connectOrBind;
}

UCI::ConnectOrBind Subscriber::getConnectOrBind() const noexcept
{
    return pImpl->mConnectOrBind;
}

UCI::SocketType Subscriber::getSocketType() noexcept
{
    return UCI::SocketType::Subscriber;
}
