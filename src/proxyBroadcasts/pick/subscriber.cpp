#include <iostream>
#include "umps/proxyBroadcasts/pick/subscriber.hpp"
#include "umps/proxyBroadcasts/pick/subscriberOptions.hpp"
#include "umps/messageFormats/pick.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/messageFormats/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyBroadcasts::Pick;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;
namespace UMF = UMPS::MessageFormats;

class Subscriber::SubscriberImpl
{
public:
    SubscriberImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        mSubscriber = std::make_unique<UPubSub::Subscriber> (context, logger);
    }
    std::unique_ptr<UPubSub::Subscriber> mSubscriber;
    SubscriberOptions mOptions;
};

/// C'tor
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
{
}

/// C'tor
Subscriber::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<SubscriberImpl> (context, nullptr))
{
}

/// C'tor
Subscriber::Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, logger))
{
}

/// C'tor
Subscriber::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context,
                       std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (context, logger))
{
}

/// Move c'tor
Subscriber::Subscriber(Subscriber &&subscriber) noexcept
{
    *this = std::move(subscriber);
}

/// Move assignment
Subscriber& Subscriber::operator=(Subscriber &&subscriber) noexcept
{
    if (&subscriber == this){return *this;}
    pImpl = std::move(subscriber.pImpl);
    return *this;
}

/// Initialize
void Subscriber::initialize(const SubscriberOptions &options)
{
    if (!options.haveAddress()){throw std::runtime_error("Address not set");}
    pImpl->mOptions = options;
    auto subscriberOptions = pImpl->mOptions.getSubscriberOptions();
    pImpl->mSubscriber->initialize(subscriberOptions);
}

/// Initialized?
bool Subscriber::isInitialized() const noexcept
{
    return pImpl->mSubscriber->isInitialized();
}

UCI::SocketDetails::Subscriber Subscriber::getSocketDetails() const
{
    return pImpl->mSubscriber->getSocketDetails();
}

/// Destructor
Subscriber::~Subscriber() = default;

/// Receive
std::unique_ptr<UMPS::MessageFormats::Pick> Subscriber::receive() const
{
    auto pick = UMF::static_unique_pointer_cast<UMPS::MessageFormats::Pick>
                (pImpl->mSubscriber->receive());
    return pick;
}
