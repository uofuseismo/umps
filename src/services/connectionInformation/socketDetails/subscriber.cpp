#include <string>
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Subscriber::SubscriberImpl
{
public:
    std::string mAddress;
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

UCI::ConnectOrBind Subscriber::connectOrBind() noexcept
{
    return UCI::ConnectOrBind::BIND;
}

UCI::SocketType Subscriber::getSocketType() noexcept
{
    return UCI::SocketType::SUBSCRIBER;
}
