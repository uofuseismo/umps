#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;

class Proxy::ProxyImpl
{
public:
    std::pair<XSubscriber, XPublisher> mXSubXPubSocketPair;
    std::pair<Router, Dealer> mRouterDealerSocketPair;
    std::pair<Router, Router> mRouterRouterSocketPair;
    UCI::SocketType mFrontendSocket{UCI::SocketType::Unknown};
    UCI::SocketType mBackendSocket{UCI::SocketType::Unknown};
    bool mHavePair{false};
};

/// C'tor
Proxy::Proxy() :
    pImpl(std::make_unique<ProxyImpl> ())
{
}

/// Copy c'tor
Proxy::Proxy(const Proxy &proxy)
{
    *this = proxy;
}

/// Move c'tor
Proxy::Proxy(Proxy &&proxy) noexcept
{
    *this = std::move(proxy);
}

/// Copy assignment
Proxy& Proxy::operator=(const Proxy &proxy)
{
    if (&proxy == this){return *this;}
    pImpl = std::make_unique<ProxyImpl> (*proxy.pImpl);
    return *this;
}

/// Move assignment
Proxy& Proxy::operator=(Proxy &&proxy) noexcept
{
    if (&proxy == this){return *this;}
    pImpl = std::move(proxy.pImpl);
    return *this;
}

/// Destructor
Proxy::~Proxy() = default;

/// Clear
void Proxy::clear() noexcept
{
    pImpl = std::make_unique<ProxyImpl> ();
}

/// Socket pair
void Proxy::setSocketPair(const std::pair<XSubscriber, XPublisher> &socketPair)
{
    if (!socketPair.first.haveAddress())
    {
        throw std::invalid_argument("xsub address not set");
    }
    if (!socketPair.second.haveAddress())
    { 
        throw std::invalid_argument("xpub address not set");
    }
    pImpl->mXSubXPubSocketPair = socketPair;
    pImpl->mFrontendSocket = socketPair.first.getSocketType();
    pImpl->mBackendSocket = socketPair.second.getSocketType();
    pImpl->mHavePair = true;
}

void Proxy::setSocketPair(const std::pair<Router, Dealer> &socketPair)
{
    if (!socketPair.first.haveAddress())
    {
        throw std::invalid_argument("Router address not set");
    }   
    if (!socketPair.second.haveAddress())
    {
        throw std::invalid_argument("Dealer address not set");
    }
    pImpl->mRouterDealerSocketPair = socketPair;
    pImpl->mFrontendSocket = socketPair.first.getSocketType();
    pImpl->mBackendSocket = socketPair.second.getSocketType();
    pImpl->mHavePair = true;
}

void Proxy::setSocketPair(const std::pair<Router, Router> &socketPair)
{
    if (!socketPair.first.haveAddress())
    {   
        throw std::invalid_argument("Router frontend address not set");
    }   
    if (!socketPair.second.haveAddress())
    {
        throw std::invalid_argument("Router backend address not set");
    }
    pImpl->mRouterRouterSocketPair = socketPair;
    pImpl->mFrontendSocket = socketPair.first.getSocketType();
    pImpl->mBackendSocket  = socketPair.second.getSocketType();
    pImpl->mHavePair = true;
}

/// Have socket pair?
bool Proxy::haveSocketPair() const noexcept
{
    return pImpl->mHavePair;
}

/// Frontend socket type
UCI::SocketType Proxy::getFrontendSocketType() const
{
    if (!haveSocketPair()){throw std::runtime_error("Socket pair not set");}
    return pImpl->mFrontendSocket;
}

/// Backend socket type
UCI::SocketType Proxy::getBackendSocketType() const
{
    if (!haveSocketPair()){throw std::runtime_error("Socket pair not set");}
    return pImpl->mBackendSocket;
}

/// Frontend sockets
XSubscriber Proxy::getXSubscriberFrontend() const
{
    if (getFrontendSocketType() != UCI::SocketType::XSubscriber)
    {
        throw std::invalid_argument("Frontend not XSUB");
    }
    return pImpl->mXSubXPubSocketPair.first;
}

/// Frontend sockets
Router Proxy::getRouterFrontend() const
{
    if (getFrontendSocketType() != UCI::SocketType::Router)
    {
        throw std::invalid_argument("Frontend not ROUTER");
    }
    if (getBackendSocketType() == UCI::SocketType::Dealer)
    {
        return pImpl->mRouterDealerSocketPair.first;
    }
    else
    {
        return pImpl->mRouterRouterSocketPair.first;
    }
}

/// Backend sockets
XPublisher Proxy::getXPublisherBackend() const
{
    if (getBackendSocketType() != UCI::SocketType::XPublisher)
    {
        throw std::invalid_argument("Backend not XPUB");
    }
    return pImpl->mXSubXPubSocketPair.second;
}

Dealer Proxy::getDealerBackend() const
{
    if (getBackendSocketType() != UCI::SocketType::Dealer)
    {
        throw std::invalid_argument("Backend not DEALER");
    }
    return pImpl->mRouterDealerSocketPair.second;
}

Router Proxy::getRouterBackend() const
{
    if (getBackendSocketType() != UCI::SocketType::Router)
    {
        throw std::invalid_argument("Backend not ROUTER");
    }
    return pImpl->mRouterRouterSocketPair.second;
}

/// Socket type
UCI::SocketType Proxy::getSocketType() noexcept
{
    return UCI::SocketType::Proxy;
}

/// Frontend socket address
std::string Proxy::getFrontendAddress() const
{
    if (getFrontendSocketType() == UCI::SocketType::XSubscriber)
    {
        return getXSubscriberFrontend().getAddress();
    }
    else if (getFrontendSocketType() == UCI::SocketType::Router)
    {
        return getRouterFrontend().getAddress();
    }
    else
    {
        throw std::runtime_error("Unhandled frontend socket type");
    }
} 

/// Backend socket address
std::string Proxy::getBackendAddress() const
{
    if (getBackendSocketType() == UCI::SocketType::XPublisher)
    {
        return getXPublisherBackend().getAddress();
    }   
    else if (getBackendSocketType() == UCI::SocketType::Dealer)
    {
        return getDealerBackend().getAddress();
    }
    else if (getBackendSocketType() == UCI::SocketType::Router)
    {
        return getRouterBackend().getAddress();
    }
    else
    {
        throw std::runtime_error("Unhandled backend socket type");
    }
}
