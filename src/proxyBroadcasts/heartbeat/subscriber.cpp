#include <zmq.hpp>
#include "umps/proxyBroadcasts/heartbeat/subscriber.hpp"
#include "umps/proxyBroadcasts/heartbeat/subscriberOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/status.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyBroadcasts::Heartbeat;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;

class Subscriber::SubscriberImpl
{
public:
    SubscriberImpl(std::shared_ptr<zmq::context_t> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
        }
        else
        {
            mContext = context;
        }
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mSubscriber = std::make_unique<UPubSub::Subscriber> (context, logger);
    }
    std::shared_ptr<zmq::context_t> mContext;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::unique_ptr<UPubSub::Subscriber> mSubscriber;
    SubscriberOptions mOptions;
};

/// C'tor
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
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
    auto subscriberOptions = options.getSubscriberOptions();
    pImpl->mSubscriber->initialize(subscriberOptions);
}

/// Initialized?
bool Subscriber::isInitialized() const noexcept
{
    return pImpl->mSubscriber->isInitialized();
}

/*
/// End point
std::string Subscriber::getEndPoint() const
{
    return pImpl->mSubscriber->getEndPoint();
}

/// Security Level
UAuth::SecurityLevel Subscriber::getSecurityLevel() const noexcept
{
    return pImpl->mSubscriber->getSecurityLevel();
}
*/
UCI::SocketDetails::Subscriber Subscriber::getSocketDetails() const
{
    return pImpl->mSubscriber->getSocketDetails();
}

/// Destructor
Subscriber::~Subscriber() = default;

/// Receive
std::unique_ptr<Status> Subscriber::receive() const
{
    auto status
        = static_unique_pointer_cast<Status> (pImpl->mSubscriber->receive());
    return status;
}

