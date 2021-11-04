#include <string>
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "private/isEmpty.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;


class Router::RouterImpl
{
public:
    std::string mAddress;
};

/// C'tor
Router::Router() :
    pImpl(std::make_unique<RouterImpl> ())
{
}

/// Copy c'tor
Router::Router(const Router &socket)
{
    *this = socket;
}

/// Move c'tor
Router::Router(Router &&socket) noexcept
{
    *this = std::move(socket);
}

/// Copy assignment
Router& Router::operator=(const Router &socket)
{
    if (&socket == this){return *this;}
    pImpl = std::make_unique<RouterImpl> (*socket.pImpl);
    return *this;
}

/// Move assignment
Router& Router::operator=(Router &&socket) noexcept
{
    if (&socket == this){return *this;}
    pImpl = std::move(socket.pImpl);
    return *this;
}

/// Destructor
Router::~Router() = default;

/// Reset class
void Router::clear() noexcept
{
    pImpl = std::make_unique<RouterImpl> ();
}

/// Address
void Router::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string Router::getAddress() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mAddress;
}

bool Router::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

UCI::ConnectOrBind Router::connectOrBind() noexcept
{
    return UCI::ConnectOrBind::CONNECT;
}

UCI::SocketType Router::getSocketType() noexcept
{
    return UCI::SocketType::ROUTER;
}
