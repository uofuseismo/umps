#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"

namespace UCI = UMPS::Services::ConnectionInformation;
using namespace UMPS::Services::ConnectionInformation::SocketDetails;

class Proxy::ProxyImpl
{
public:
    std::pair<XSubscriber, XPublisher> mXSubXPubSocketPair;
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

/// Backend sockets
XPublisher Proxy::getXPublisherBackend() const
{
    if (getBackendSocketType() != UCI::SocketType::XPUBLISHER)
    {
        throw std::invalid_argument("Frontend not XPUB");
    }
    return pImpl->mXSubXPubSocketPair.second;
}

/// Socket type
UCI::SocketType Proxy::getSocketType() noexcept
{
    return UCI::SocketType::PROXY;
}
