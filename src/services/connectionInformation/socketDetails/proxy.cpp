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
    UCI::SocketType mFrontendSocket = UCI::SocketType::UNKNOWN;
    UCI::SocketType mBackendSocket = UCI::SocketType::UNKNOWN;
    bool mHavePair = false;
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
        throw std::invalid_argument("router address not set");
    }   
    if (!socketPair.second.haveAddress())
    {
        throw std::invalid_argument("dealer address not set");
    }
    pImpl->mRouterDealerSocketPair = socketPair;
    pImpl->mFrontendSocket = socketPair.first.getSocketType();
    pImpl->mBackendSocket = socketPair.second.getSocketType();
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
    if (getFrontendSocketType() != UCI::SocketType::XSUBSCRIBER)
    {
        throw std::invalid_argument("Frontend not XSUB");
    }
    return pImpl->mXSubXPubSocketPair.first;
}

/// Frontend sockets
Router Proxy::getRouterFrontend() const
{
    if (getFrontendSocketType() != UCI::SocketType::ROUTER)
    {
        throw std::invalid_argument("Frontend not ROUTER");
    }
    return pImpl->mRouterDealerSocketPair.first;
}

/// Backend sockets
XPublisher Proxy::getXPublisherBackend() const
{
    if (getBackendSocketType() != UCI::SocketType::XPUBLISHER)
    {
        throw std::invalid_argument("Backend not XPUB");
    }
    return pImpl->mXSubXPubSocketPair.second;
}

Dealer Proxy::getDealerBackend() const
{
    if (getBackendSocketType() != UCI::SocketType::DEALER)
    {
        throw std::invalid_argument("Backend not DEALER");
    }
    return pImpl->mRouterDealerSocketPair.second;
}

/// Socket type
UCI::SocketType Proxy::getSocketType() noexcept
{
    return UCI::SocketType::PROXY;
}

/// Frontend socket address
std::string Proxy::getFrontendAddress() const
{
    if (getFrontendSocketType() == UCI::SocketType::XSUBSCRIBER)
    {
        return getXSubscriberFrontend().getAddress();
    }
    else if (getFrontendSocketType() == UCI::SocketType::ROUTER)
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
    if (getBackendSocketType() == UCI::SocketType::XPUBLISHER)
    {
        return getXPublisherBackend().getAddress();
    }   
    else if (getBackendSocketType() == UCI::SocketType::DEALER)
    {
        return getDealerBackend().getAddress();
    }
    else
    {
        throw std::runtime_error("Unhandled backend socket type");
    }
}
