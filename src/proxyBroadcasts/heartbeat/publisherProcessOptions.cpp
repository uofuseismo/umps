#include <iostream>
#include <string>
#include <chrono>
#include "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisher.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyBroadcasts::Heartbeat;
namespace UCI = UMPS::Services::ConnectionInformation;

class PublisherProcessOptions::PublisherProcessOptionsImpl
{
public:
    std::chrono::seconds mInterval{30};
};

/// C'tor
PublisherProcessOptions::PublisherProcessOptions() :
    pImpl(std::make_unique<PublisherProcessOptionsImpl> ())
{
}

/// Copy c'tor
PublisherProcessOptions::PublisherProcessOptions(
    const PublisherProcessOptions &options)
{
    *this = options;
}

/// Move c'tor
PublisherProcessOptions::PublisherProcessOptions(
    PublisherProcessOptions &&options) noexcept
{
    *this = std::move(options);
} 

/// Copy assignment
PublisherProcessOptions& PublisherProcessOptions::operator=(
    const PublisherProcessOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<PublisherProcessOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
PublisherProcessOptions& PublisherProcessOptions::operator=(
    PublisherProcessOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
PublisherProcessOptions::~PublisherProcessOptions() = default;

/// Reset class
void PublisherProcessOptions::clear() noexcept
{
    pImpl = std::make_unique<PublisherProcessOptionsImpl> ();
}

/// Heartbeat interval
void PublisherProcessOptions::setInterval(const std::chrono::seconds &interval)
{
    if (interval.count() <= 0)
    {
        throw std::invalid_argument("Heartbeat interval must be positive");
    }
    pImpl->mInterval = interval;
}

std::chrono::seconds PublisherProcessOptions::getInterval() const noexcept
{
    return pImpl->mInterval;
}
