#include "umps/services/command/remoteReplierOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"

using namespace UMPS::Services::Command;
namespace UAuth = UMPS::Authentication;
namespace URouterDealer = UMPS::Messaging::RouterDealer;

class RemoteReplierOptions::RemoteReplierOptionsImpl
{
public:
    URouterDealer::ReplyOptions mOptions;
};

/// C'tor
RemoteReplierOptions::RemoteReplierOptions() :
    pImpl(std::make_unique<RemoteReplierOptionsImpl> ())
{
}

/// Copy c'tor
RemoteReplierOptions::RemoteReplierOptions(const RemoteReplierOptions &options)
{
    *this = options;
}

/// Move c'tor
RemoteReplierOptions::RemoteReplierOptions(
    RemoteReplierOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RemoteReplierOptions&
RemoteReplierOptions::operator=(const RemoteReplierOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RemoteReplierOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RemoteReplierOptions&
RemoteReplierOptions::operator=(RemoteReplierOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset class
void RemoteReplierOptions::clear() noexcept
{
    pImpl = std::make_unique<RemoteReplierOptionsImpl> ();
}

/// Destructor
RemoteReplierOptions::~RemoteReplierOptions() = default;

/// Set address
void RemoteReplierOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

bool RemoteReplierOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// Callback
void RemoteReplierOptions::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mOptions.setCallback(callback);
}

bool RemoteReplierOptions::haveCallback() const noexcept
{
    return pImpl->mOptions.haveCallback();
}

/// ZAP options
void RemoteReplierOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
}

/// HWM
void RemoteReplierOptions::setHighWaterMark(const int hwm)
{
    pImpl->mOptions.setHighWaterMark(hwm);
}

/// Options
URouterDealer::ReplyOptions RemoteReplierOptions::getOptions() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    if (!haveCallback()){throw std::runtime_error("Callback not set");}
    return pImpl->mOptions;
}
