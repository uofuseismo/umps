#include <string>
#include <chrono>
#include "umps/messaging/publisherSubscriber/publisherOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;
namespace UAuth = UMPS::Authentication;

class PublisherOptions::PublisherOptionsImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::string mAddress;
    std::chrono::milliseconds mTimeOut{-1}; // Wait forever
    int mHighWaterMark = 0;
};

/// C'tor
PublisherOptions::PublisherOptions() :
    pImpl(std::make_unique<PublisherOptionsImpl> ())
{
}

/// Copy c'tor
PublisherOptions::PublisherOptions(const PublisherOptions &options)
{
    *this = options;
}

/// Move c'tor
PublisherOptions::PublisherOptions(PublisherOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Destructor
PublisherOptions::~PublisherOptions() = default;

/// Reset class
void PublisherOptions::clear() noexcept
{
    pImpl = std::make_unique<PublisherOptionsImpl> ();
}

/// Copy assignment
PublisherOptions&
    PublisherOptions::operator=(const PublisherOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<PublisherOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
PublisherOptions&
    PublisherOptions::operator=(PublisherOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// High water mark
void PublisherOptions::setSendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mHighWaterMark = hwm;
}

int PublisherOptions::getSendHighWaterMark() const noexcept
{
    return pImpl->mHighWaterMark;
}

/// Address
void PublisherOptions::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mAddress = address;
}

std::string PublisherOptions::getAddress() const
{
    if (!haveAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    return pImpl->mAddress;
}

bool PublisherOptions::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// ZAP options
void PublisherOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
} 

UAuth::ZAPOptions PublisherOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// Timeout
void PublisherOptions::setSendTimeOut(
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

std::chrono::milliseconds PublisherOptions::getSendTimeOut() const noexcept
{
    return pImpl->mTimeOut;
}
