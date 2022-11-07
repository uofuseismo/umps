#include <string>
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;
namespace UAuth = UMPS::Authentication;

class SubscriberOptions::SubscriberOptionsImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    UMPS::MessageFormats::Messages mMessageTypes;
    std::string mAddress;
    std::chrono::milliseconds mTimeOut{-1};
    int mHighWaterMark = 0;
};

/// C'tor
SubscriberOptions::SubscriberOptions() :
    pImpl(std::make_unique<SubscriberOptionsImpl> ())
{
}

/// Copy c'tor
SubscriberOptions::SubscriberOptions(const SubscriberOptions &options)
{
    *this = options;
}

/// Move c'tor
SubscriberOptions::SubscriberOptions(SubscriberOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Destructor
SubscriberOptions::~SubscriberOptions() = default;

/// Reset class
void SubscriberOptions::clear() noexcept
{
    pImpl = std::make_unique<SubscriberOptionsImpl> ();
}

/// Copy assignment
SubscriberOptions&
    SubscriberOptions::operator=(const SubscriberOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<SubscriberOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
SubscriberOptions&
    SubscriberOptions::operator=(SubscriberOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// High water mark
void SubscriberOptions::setReceiveHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mHighWaterMark = hwm;
}

int SubscriberOptions::getReceiveHighWaterMark() const noexcept
{
    return pImpl->mHighWaterMark;
}

/// Address
void SubscriberOptions::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string SubscriberOptions::getAddress() const
{
    if (!haveAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    return pImpl->mAddress;
}

bool SubscriberOptions::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// ZAP options
void SubscriberOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
} 

UAuth::ZAPOptions SubscriberOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// Timeout
void SubscriberOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    constexpr std::chrono::milliseconds zero{0};
    if (timeOut >= zero)
    {
        pImpl->mTimeOut = timeOut;
    }
    else
    {
        pImpl->mTimeOut = std::chrono::milliseconds{-1};
    }
}

std::chrono::milliseconds SubscriberOptions::getReceiveTimeOut() const noexcept
{
    return pImpl->mTimeOut;
}

/// Message types
void SubscriberOptions::setMessageTypes(
    const UMPS::MessageFormats::Messages &messageTypes)
{
    if (messageTypes.empty())
    {
        throw std::invalid_argument("No message type set");
    }
    pImpl->mMessageTypes = messageTypes;
}

UMPS::MessageFormats::Messages SubscriberOptions::getMessageTypes() const
{
    if (!haveMessageTypes())
    {
        throw std::invalid_argument("Message types not yet set");
    }
    return pImpl->mMessageTypes;
}

bool SubscriberOptions::haveMessageTypes() const noexcept
{
    return !pImpl->mMessageTypes.empty();
}
