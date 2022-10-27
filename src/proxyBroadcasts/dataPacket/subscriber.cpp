#include <iostream>
#include "umps/proxyBroadcasts/dataPacket/subscriber.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyBroadcasts::DataPacket;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;

template<class T>
class Subscriber<T>::SubscriberImpl
{
public:
    /*
    SubscriberImpl(std::shared_ptr<zmq::context_t> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger, int)
    {
        mSubscriber = std::make_unique<UPubSub::Subscriber> (context, logger);
    }
    */
    SubscriberImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        mSubscriber = std::make_unique<UPubSub::Subscriber> (context, logger);
    }
    std::unique_ptr<UPubSub::Subscriber> mSubscriber;
    SubscriberOptions<T> mOptions;
};

/// C'tor
template<class T>
Subscriber<T>::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
{
}

/*
template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<SubscriberImpl> (context, nullptr, 0))
{
}
*/

template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<SubscriberImpl> (context, nullptr))
{
}

template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, logger))
{
}

/*
template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<zmq::context_t> &context,
                          std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (context, logger, 0))
{
}
*/

template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context,
                          std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (context, logger))
{
}

/// Move c'tor
template<class T>
Subscriber<T>::Subscriber(Subscriber &&subscriber) noexcept
{
    *this = std::move(subscriber);
}

/// Move assignment
template<class T>
Subscriber<T>& Subscriber<T>::operator=(Subscriber &&subscriber) noexcept
{
    if (&subscriber == this){return *this;}
    pImpl = std::move(subscriber.pImpl);
    return *this;
}

/// Initialize
template<class T>
void Subscriber<T>::initialize(const SubscriberOptions<T> &options)
{
    if (!options.haveAddress()){throw std::runtime_error("Address not set");}
    pImpl->mOptions = options;
    auto subscriberOptions = pImpl->mOptions.getSubscriberOptions();
    pImpl->mSubscriber->initialize(subscriberOptions);
}

/// Initialized?
template<class T>
bool Subscriber<T>::isInitialized() const noexcept
{
    return pImpl->mSubscriber->isInitialized();
}

/*
/// End point
template<class T>
std::string Subscriber<T>::getEndPoint() const
{
    return pImpl->mSubscriber->getEndPoint();
}

/// Security Level
template<class T>
UAuth::SecurityLevel Subscriber<T>::getSecurityLevel() const noexcept
{
    return pImpl->mSubscriber->getSecurityLevel();
}
*/
template<class T>
UCI::SocketDetails::Subscriber Subscriber<T>::getSocketDetails() const
{
    return pImpl->mSubscriber->getSocketDetails();
}

/// Destructor
template<class T>
Subscriber<T>::~Subscriber() = default;

/// Receive
template<class T>
std::unique_ptr<UMPS::MessageFormats::DataPacket<T>>
    Subscriber<T>::receive() const
{
    auto dataPacket
        = static_unique_pointer_cast<UMPS::MessageFormats::DataPacket<T>>
          (pImpl->mSubscriber->receive());
    return dataPacket;
}

///--------------------------------------------------------------------------///
///                             Template Instantiation                       ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyBroadcasts::DataPacket::Subscriber<double>;
template class UMPS::ProxyBroadcasts::DataPacket::Subscriber<float>;
template class UMPS::ProxyBroadcasts::DataPacket::Subscriber<int>;
