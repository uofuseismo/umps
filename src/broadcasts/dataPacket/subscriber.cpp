#include <zmq.hpp>
#include "umps/broadcasts/dataPacket/subscriber.hpp"
#include "umps/broadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/authentication/service.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Broadcasts::DataPacket;
namespace UAuth = UMPS::Authentication;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;

template<class T>
class Subscriber<T>::SubscriberImpl
{
public:
    SubscriberImpl(std::shared_ptr<zmq::context_t> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger)
    //               std::shared_ptr<UAuth::IAuthenticator> authenticator)
    {
        mSubscriber = std::make_unique<UPubSub::Subscriber> (context, logger);
    }
    //std::shared_ptr<UAuth::IAuthenticator> mAuthenticator;
    std::unique_ptr<UPubSub::Subscriber> mSubscriber;
    SubscriberOptions<T> mOptions;
};

/// C'tor
template<class T>
Subscriber<T>::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
{
}

/// C'tor
template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<SubscriberImpl> (context, nullptr))
{
}

/// C'tor
template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, logger))
{
}

/// C'tor
template<class T>
Subscriber<T>::Subscriber(std::shared_ptr<zmq::context_t> &context,
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
    auto subscriberOptions = options.getSubscriberOptions();
    pImpl->mSubscriber->initialize(subscriberOptions);
    pImpl->mOptions = options;
}

/// Initialized?
template<class T>
bool Subscriber<T>::isInitialized() const noexcept
{
    return pImpl->mSubscriber->isInitialized();
}

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
template class UMPS::Broadcasts::DataPacket::Subscriber<double>;
template class UMPS::Broadcasts::DataPacket::Subscriber<float>;
template class UMPS::Broadcasts::DataPacket::Subscriber<int>;
