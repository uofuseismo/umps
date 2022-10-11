#include <chrono>
#include <algorithm>
#include <string>
#include <filesystem>
#include "umps/proxyServices/command/requestorOptions.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"

using namespace UMPS::ProxyServices::Command;

class RequestorOptions::RequestorOptionsImpl
{
public:
    RequestorOptionsImpl()
    {
        mOptions.setTimeOut(std::chrono::milliseconds {1000});
    }
    UMPS::Messaging::RequestRouter::RequestOptions mOptions;
};

/// C'tor
RequestorOptions::RequestorOptions() :
    pImpl(std::make_unique<RequestorOptionsImpl> ())
{
}

/// Copy c'tor
RequestorOptions::RequestorOptions(
    const RequestorOptions &options)
{
    *this = options;
}

/// Copy m'tor
RequestorOptions::RequestorOptions(
    RequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RequestorOptions&
RequestorOptions::operator=(const RequestorOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RequestorOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RequestorOptions&
RequestorOptions::operator=(RequestorOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
RequestorOptions::~RequestorOptions() = default;

/// Reset
void RequestorOptions::clear() noexcept
{
    pImpl = std::make_unique<RequestorOptionsImpl> ();
} 

/// Address
void RequestorOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

bool RequestorOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// Time-out
void RequestorOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut)
{
    pImpl->mOptions.setTimeOut(timeOut);
} 

UMPS::Messaging::RequestRouter::RequestOptions 
RequestorOptions::getOptions() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mOptions;
}
