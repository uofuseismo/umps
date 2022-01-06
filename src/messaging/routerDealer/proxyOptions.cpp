#include <string>
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UAuth = UMPS::Authentication;

class ProxyOptions::ProxyOptionsImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::string mBackendAddress;
    std::string mFrontendAddress;
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

/// ZAP options
void ProxyOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
} 

UAuth::ZAPOptions ProxyOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}
