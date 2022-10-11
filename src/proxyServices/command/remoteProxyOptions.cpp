#include "umps/proxyServices/command/remoteProxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UAuth = UMPS::Authentication;

class RemoteProxyOptions::RemoteProxyOptionsImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    int mBackendHighWaterMark{0};
    int mFrontendHighWaterMark{0};
};

/// C'tor
RemoteProxyOptions::RemoteProxyOptions() :
    pImpl(std::make_unique<RemoteProxyOptionsImpl> ())
{
}

/// Copy c'tor
RemoteProxyOptions::RemoteProxyOptions(const RemoteProxyOptions &options)
{
    *this = options;
}

/// Move c'tor
RemoteProxyOptions::RemoteProxyOptions(RemoteProxyOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RemoteProxyOptions&
RemoteProxyOptions::operator=(const RemoteProxyOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RemoteProxyOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RemoteProxyOptions&
RemoteProxyOptions::operator=(RemoteProxyOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset
void RemoteProxyOptions::clear() noexcept
{
    pImpl = std::make_unique<RemoteProxyOptionsImpl> ();
}

/// Destructor
RemoteProxyOptions::~RemoteProxyOptions() = default;

/// Frontend HWM
void RemoteProxyOptions::setFrontendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mFrontendHighWaterMark = hwm;
}

int RemoteProxyOptions::getFrontendHighWaterMark() const noexcept
{
    return pImpl->mFrontendHighWaterMark;
}

/// Backend HWM
void RemoteProxyOptions::setBackendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mBackendHighWaterMark = hwm;
}

int RemoteProxyOptions::getBackendHighWaterMark() const noexcept
{
    return pImpl->mBackendHighWaterMark;
}

/// Frontend address
void RemoteProxyOptions::setFrontendAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mFrontendAddress = address;
}

std::string RemoteProxyOptions::getFrontendAddress() const
{
    if (!haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    return pImpl->mFrontendAddress;
}

bool RemoteProxyOptions::haveFrontendAddress() const noexcept
{
    return !pImpl->mFrontendAddress.empty();
}

/// Backend address
void RemoteProxyOptions::setBackendAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    if (pImpl->mFrontendAddress == address)
    {
        throw std::invalid_argument(
            "Backend address cannot match frontend address");
    }
    pImpl->mBackendAddress = address;
}

std::string RemoteProxyOptions::getBackendAddress() const
{
    if (!haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    return pImpl->mBackendAddress;
}

bool RemoteProxyOptions::haveBackendAddress() const noexcept
{
    return !pImpl->mBackendAddress.empty();
}

/// ZAP options
void RemoteProxyOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
} 

UAuth::ZAPOptions RemoteProxyOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

