#include <string>
#include "umps/messaging/xPublisherXSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;
namespace UAuth = UMPS::Authentication;

class SubscriberOptions::SubscriberOptionsImpl
{
public:
    UMPS::Messaging::PublisherSubscriber::SubscriberOptions mOptions;
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
    pImpl->mOptions.setReceiveHighWaterMark(hwm);
}

int SubscriberOptions::getReceiveHighWaterMark() const noexcept
{
    return pImpl->mOptions.getReceiveHighWaterMark();
}

/// Address
void SubscriberOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

std::string SubscriberOptions::getAddress() const
{
    return pImpl->mOptions.getAddress();
}

bool SubscriberOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// ZAP options
void SubscriberOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
} 

UAuth::ZAPOptions SubscriberOptions::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// Timeout
void SubscriberOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->mOptions.setReceiveTimeOut(timeOut);
}

std::chrono::milliseconds SubscriberOptions::getReceiveTimeOut() const noexcept
{
    return pImpl->mOptions.getReceiveTimeOut();
}

/// Message types
void SubscriberOptions::setMessageTypes(
    const UMPS::MessageFormats::Messages &messageTypes)
{
    pImpl->mOptions.setMessageTypes(messageTypes);
}

UMPS::MessageFormats::Messages SubscriberOptions::getMessageTypes() const
{
    return pImpl->mOptions.getMessageTypes();
}

bool SubscriberOptions::haveMessageTypes() const noexcept
{
    return pImpl->mOptions.haveMessageTypes();
}
