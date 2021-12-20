#include <zmq.hpp>
#include "umps/broadcasts/heartbeat/subscriber.hpp"
#include "umps/broadcasts/heartbeat/subscriberOptions.hpp"
#include "umps/broadcasts/heartbeat/status.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/authentication/service.hpp"
#include "umps/messaging/authentication/authenticator.hpp"
#include "umps/messaging/authentication/grasslands.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Broadcasts::Heartbeat;
namespace UAuth = UMPS::Messaging::Authentication;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;

class Subscriber::SubscriberImpl
{
public:
    SubscriberImpl(std::shared_ptr<zmq::context_t> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger)
    //               std::shared_ptr<UAuth::IAuthenticator> authenticator)
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
        //if (authenticator == nullptr)
        //{
        //    mAuthenticator = std::make_shared<UAuth::Grasslands> (logger);
        //}
        mSubscriber = std::make_unique<UPubSub::Subscriber> (context, logger);
    }
    std::shared_ptr<zmq::context_t> mContext;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator;
    std::unique_ptr<UPubSub::Subscriber> mSubscriber;
    SubscriberOptions mOptions;
};

/// C'tor
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
{
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

/// Destructor
Subscriber::~Subscriber() = default;

/// Receive
std::unique_ptr<Status> Subscriber::receive() const
{
    auto status
        = static_unique_pointer_cast<Status> (pImpl->mSubscriber->receive());
    return status;
}

