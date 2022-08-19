#include <string>
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Reply::ReplyImpl
{
public:
    std::string mAddress;
    UCI::ConnectOrBind mConnectOrBind{UCI::ConnectOrBind::Bind};
    UAuth::SecurityLevel mSecurityLevel{UAuth::SecurityLevel::Grasslands};
    UAuth::UserPrivileges mUserPrivileges{UAuth::UserPrivileges::ReadOnly};
};

/// C'tor
Reply::Reply() :
    pImpl(std::make_unique<ReplyImpl> ())
{
}

/// Copy c'tor
Reply::Reply(const Reply &socket)
{
    *this = socket;
}

/// Move c'tor
Reply::Reply(Reply &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
Reply& Reply::operator=(const Reply &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<ReplyImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
Reply& Reply::operator=(Reply &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
Reply::~Reply() = default;

/// Reset class
void Reply::clear() noexcept
{
    pImpl = std::make_unique<ReplyImpl> ();
}

/// Address
void Reply::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string Reply::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool Reply::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Securtiy level
void Reply::setSecurityLevel(
    const UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel Reply::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}


/// Connect or bind
void Reply::setConnectOrBind(
    const UCI::ConnectOrBind connectOrBind) noexcept
{
    pImpl->mConnectOrBind = connectOrBind;
}

UCI::ConnectOrBind Reply::getConnectOrBind() const noexcept
{
    return pImpl->mConnectOrBind;
}

UCI::SocketType Reply::getSocketType() noexcept
{
    return UCI::SocketType::Reply;
}

/// Privileges
void Reply::setMinimumUserPrivileges(
    const UAuth::UserPrivileges privileges) noexcept
{
    pImpl->mUserPrivileges = privileges;
}

UAuth::UserPrivileges Reply::getMinimumUserPrivileges() const noexcept
{
    return pImpl->mUserPrivileges;
}

