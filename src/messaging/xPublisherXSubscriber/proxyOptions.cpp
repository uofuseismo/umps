#include <string>
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;

class ProxyOptions::ProxyOptionsImpl
{
public:
    UMPS::Messaging::Authentication::ZAPOptions mZAPOptions;
    std::string mBackendAddress;
    std::string mFrontendAddress;
    std::string mTopic;
    int mBackendHighWaterMark = 0;
    int mFrontendHighWaterMark = 0;
};

/// C'tor
ProxyOptions::ProxyOptions() :
    pImpl(std::make_unique<ProxyOptionsImpl> ())
{
}

/// Copy c'tor
ProxyOptions::ProxyOptions(const ProxyOptions &options)
{
    *this = options;
}

/// Move c'tor
ProxyOptions::ProxyOptions(ProxyOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Destructor
ProxyOptions::~ProxyOptions() = default;

/// Reset class
void ProxyOptions::clear() noexcept
{
    pImpl = std::make_unique<ProxyOptionsImpl> ();
}

/// Copy assignment
ProxyOptions& ProxyOptions::operator=(const ProxyOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ProxyOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ProxyOptions& ProxyOptions::operator=(ProxyOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Frontend HWM
void ProxyOptions::setFrontendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mFrontendHighWaterMark = hwm;
}

int ProxyOptions::getFrontendHighWaterMark() const noexcept
{
    return pImpl->mFrontendHighWaterMark;
}

/// Backend HWM
void ProxyOptions::setBackendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mBackendHighWaterMark = hwm;
}

int ProxyOptions::getBackendHighWaterMark() const noexcept
{
    return pImpl->mBackendHighWaterMark;
}

/// Frontend address
void ProxyOptions::setFrontendAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mFrontendAddress = address;
}

std::string ProxyOptions::getFrontendAddress() const
{
    if (!haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    return pImpl->mFrontendAddress;
}

bool ProxyOptions::haveFrontendAddress() const noexcept
{
    return !pImpl->mFrontendAddress.empty();
}

/// Backend address
void ProxyOptions::setBackendAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mBackendAddress = address;
}

std::string ProxyOptions::getBackendAddress() const
{
    if (!haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    return pImpl->mBackendAddress;
}

bool ProxyOptions::haveBackendAddress() const noexcept
{
    return !pImpl->mBackendAddress.empty();
}

/// Topic 
void ProxyOptions::setTopic(const std::string &topic)
{
    if (isEmpty(topic)){throw std::invalid_argument("Topic is empty");}
    pImpl->mTopic = topic;
}

std::string ProxyOptions::getTopic() const
{
    if (!haveTopic()){throw std::runtime_error("Topic not set");}
    return pImpl->mTopic;
}

bool ProxyOptions::haveTopic() const noexcept
{
    return !pImpl->mTopic.empty();
}

/// ZAP options
void ProxyOptions::setZAPOptions(
    const UMPS::Messaging::Authentication::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
} 

UMPS::Messaging::Authentication::ZAPOptions 
    ProxyOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}