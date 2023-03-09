#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/xPublisherXSubscriber/subscriber.hpp"
#include "umps/messaging/xPublisherXSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/logging/standardOut.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Subscriber::SubscriberImpl
{
public:
    /// C'tor
    SubscriberImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                   std::shared_ptr<UMPS::Logging::ILog> logger) :
        mSubscriber(context, logger)
    {
    }
//private:
    UMPS::Messaging::PublisherSubscriber::Subscriber mSubscriber;
    UCI::SocketDetails::XSubscriber mSocketDetails;
};

/// Constructors
Subscriber::Subscriber() :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, nullptr))
{
}

Subscriber::Subscriber(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<SubscriberImpl> (context, nullptr)) 
{
}

Subscriber::Subscriber(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<SubscriberImpl> (nullptr, logger))
{
}

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

/// Destructor
Subscriber::~Subscriber() = default;

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
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set on options");
    }
    if (!options.haveMessageTypes())
    {
        throw std::invalid_argument("Message types not set on options");
    }
    UMPS::Messaging::PublisherSubscriber::SubscriberOptions sOptions;
    sOptions.setZAPOptions(options.getZAPOptions());
    sOptions.setReceiveTimeOut(options.getReceiveTimeOut());
    sOptions.setReceiveHighWaterMark(options.getReceiveHighWaterMark()); 
    sOptions.setAddress(options.getAddress());
    sOptions.setMessageTypes(options.getMessageTypes());
    pImpl->mSubscriber.initialize(sOptions);
    // Reconstitute the socket details
    auto socketDetails = pImpl->mSubscriber.getSocketDetails();
    pImpl->mSocketDetails.setAddress(socketDetails.getAddress());
    pImpl->mSocketDetails.setSecurityLevel(socketDetails.getSecurityLevel());
    pImpl->mSocketDetails.setConnectOrBind(socketDetails.getConnectOrBind());
    pImpl->mSocketDetails.setMinimumUserPrivileges(
        socketDetails.getMinimumUserPrivileges());
}

/// Initialized?
bool Subscriber::isInitialized() const noexcept
{
    return pImpl->mSubscriber.isInitialized();
}

/// Disconnect from endpoint
void Subscriber::disconnect()
{
    pImpl->mSubscriber.disconnect();
}

/// Receive messages
std::unique_ptr<UMPS::MessageFormats::IMessage> Subscriber::receive() const
{
    return pImpl->mSubscriber.receive();
}
/// Socket details
UCI::SocketDetails::XSubscriber Subscriber::getSocketDetails() const
{
    return pImpl->mSocketDetails;
}
