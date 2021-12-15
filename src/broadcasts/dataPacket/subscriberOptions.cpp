#include <string>
#include "umps/broadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"

using namespace UMPS::Broadcasts::DataPacket; 
namespace UAuth = UMPS::Messaging::Authentication;

class SubscriberOptions::SubscriberOptionsImpl
{
public:
    SubscriberOptionsImpl()
    {
        mOptions.setHighWaterMark(8192);
        mOptions.setTimeOut(std::chrono::milliseconds{10});
        UMPS::MessageFormats::Messages messageTypes;
        std::unique_ptr<UMPS::MessageFormats::IMessage> dataPacketMessageType
            = std::make_unique<UMPS::MessageFormats::DataPacket<double>> (); 
        messageTypes.add(dataPacketMessageType);
        mOptions.setMessageTypes(messageTypes);
    }
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
    return !pImpl->mOptions.haveAddress();
}

/// High water mark
void SubscriberOptions::setHighWaterMark(const int hwm)
{
    pImpl->mOptions.setHighWaterMark(hwm);
}

int SubscriberOptions::getHighWaterMark() const noexcept
{
    return pImpl->mOptions.getHighWaterMark();
}

/// ZAP options
void SubscriberOptions::setZAPOptions(
    const UMPS::Messaging::Authentication::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
} 

UAuth::ZAPOptions  SubscriberOptions::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// Timeout
void SubscriberOptions::setTimeOut(
    const std::chrono::milliseconds timeOut) noexcept
{
    pImpl->mOptions.setTimeOut(timeOut);
}

std::chrono::milliseconds SubscriberOptions::getTimeOut() const noexcept
{
    return pImpl->mOptions.getTimeOut();
}

/// Gets the options
UMPS::Messaging::PublisherSubscriber::SubscriberOptions
    SubscriberOptions::getSubscriberOptions() const noexcept
{
    return pImpl->mOptions;
}
