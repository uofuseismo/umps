#include <iostream>
#include <zmq.hpp>
#include "umps/proxyBroadcasts/dataPacket/publisher.hpp"
#include "umps/proxyBroadcasts/dataPacket/publisherOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyBroadcasts::DataPacket;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace UXPubXSub = UMPS::Messaging::XPublisherXSubscriber;

class Publisher::PublisherImpl
{
public:
    PublisherImpl(std::shared_ptr<zmq::context_t> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        mPublisher = std::make_unique<UXPubXSub::Publisher> (context, logger);
    }
    std::unique_ptr<UXPubXSub::Publisher> mPublisher;
    PublisherOptions mOptions;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> (nullptr, nullptr))
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<PublisherImpl> (context, nullptr))
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (nullptr, logger))
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context,
                     std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (context, logger))
{
}

/// Move c'tor
Publisher::Publisher(Publisher &&publisher) noexcept
{
    *this = std::move(publisher);
}

/// Move assignment
Publisher& Publisher::operator=(Publisher &&publisher) noexcept
{
    if (&publisher == this){return *this;}
    pImpl = std::move(publisher.pImpl);
    return *this;
}

/// Initialize
void Publisher::initialize(const PublisherOptions &options)
{
    if (!options.haveAddress()){throw std::runtime_error("Address not set");}
    auto publisherOptions = options.getPublisherOptions();
    pImpl->mPublisher->initialize(publisherOptions);
    pImpl->mOptions = options;
}

/// Initialized?
bool Publisher::isInitialized() const noexcept
{
    return pImpl->mPublisher->isInitialized();
}

/*
/// End point
std::string Publisher::getEndPoint() const
{
    return pImpl->mPublisher->getEndPoint();
}

/// Security Level
UAuth::SecurityLevel Publisher::getSecurityLevel() const noexcept
{
    return pImpl->mPublisher->getSecurityLevel();
}
*/
UCI::SocketDetails::XPublisher Publisher::getSocketDetails() const
{
    return pImpl->mPublisher->getSocketDetails();
}

/// Destructor
Publisher::~Publisher() = default;

/// Send
template<typename U>
void Publisher::send(const UMPS::MessageFormats::DataPacket<U> &message)
{
std::cout << message.getMessageType() << std::endl;
    pImpl->mPublisher->send(message); 
}

///--------------------------------------------------------------------------///
///                             Template Instantiation                       ///
///--------------------------------------------------------------------------///
template void UMPS::ProxyBroadcasts::DataPacket::Publisher::send(
    const UMPS::MessageFormats::DataPacket<double> &message);
template void UMPS::ProxyBroadcasts::DataPacket::Publisher::send(
    const UMPS::MessageFormats::DataPacket<float> &message);
template void UMPS::ProxyBroadcasts::DataPacket::Publisher::send(
    const UMPS::MessageFormats::DataPacket<int> &message);
template void UMPS::ProxyBroadcasts::DataPacket::Publisher::send(
    const UMPS::MessageFormats::DataPacket<int16_t> &message);
