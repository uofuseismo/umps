#include <string>
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Dealer::DealerImpl
{
public:
    std::string mAddress;
};

/// C'tor
Dealer::Dealer() :
    pImpl(std::make_unique<DealerImpl> ())
{
}

/// Copy c'tor
Dealer::Dealer(const Dealer &socket)
{
    *this = socket;
}

/// Move c'tor
Dealer::Dealer(Dealer &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
Dealer& Dealer::operator=(const Dealer &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<DealerImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
Dealer& Dealer::operator=(Dealer &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
Dealer::~Dealer() = default;

/// Reset class
void Dealer::clear() noexcept
{
    pImpl = std::make_unique<DealerImpl> ();
}

/// Address
void Dealer::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string Dealer::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool Dealer::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

UCI::ConnectOrBind Dealer::connectOrBind() noexcept
{
    return UCI::ConnectOrBind::CONNECT;
}

UCI::SocketType Dealer::getSocketType() noexcept
{
    return UCI::SocketType::DEALER;
}
