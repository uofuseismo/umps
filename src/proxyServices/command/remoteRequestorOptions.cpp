#include <chrono>
#include <algorithm>
#include <string>
#include <filesystem>
#include "umps/proxyServices/command/remoteRequestorOptions.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"

using namespace UMPS::ProxyServices::Command;

class RemoteRequestorOptions::RemoteRequestorOptionsImpl
{
public:
    RemoteRequestorOptionsImpl()
    {
        mOptions.setTimeOut(std::chrono::milliseconds {1000});
    }
    UMPS::Messaging::RequestRouter::RequestOptions mOptions;
};

/// C'tor
RemoteRequestorOptions::RemoteRequestorOptions() :
    pImpl(std::make_unique<RemoteRequestorOptionsImpl> ())
{
}

/// Copy c'tor
RemoteRequestorOptions::RemoteRequestorOptions(
    const RemoteRequestorOptions &options)
{
    *this = options;
}

/// Copy m'tor
RemoteRequestorOptions::RemoteRequestorOptions(
    RemoteRequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RemoteRequestorOptions&
RemoteRequestorOptions::operator=(const RemoteRequestorOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RemoteRequestorOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RemoteRequestorOptions&
RemoteRequestorOptions::operator=(RemoteRequestorOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
RemoteRequestorOptions::~RemoteRequestorOptions() = default;

/// Reset
void RemoteRequestorOptions::clear() noexcept
{
    pImpl = std::make_unique<RemoteRequestorOptionsImpl> ();
} 

/// Address
void RemoteRequestorOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

bool RemoteRequestorOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// Time-out
void RemoteRequestorOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut)
{
    pImpl->mOptions.setTimeOut(timeOut);
} 

UMPS::Messaging::RequestRouter::RequestOptions 
RemoteRequestorOptions::getOptions() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->mOptions;
}
