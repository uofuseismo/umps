#include <map>
#include <string>
#include <functional>
#include <chrono>
#include "umps/proxyServices/packetCache/replierOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UAuth = UMPS::Authentication;

class ReplierOptions::ReplierOptionsImpl
{
public:
    UMPS::Messaging::RouterDealer::ReplyOptions mOptions;
};

/// C'tor
ReplierOptions::ReplierOptions() :
    pImpl(std::make_unique<ReplierOptionsImpl> ())
{
}

/// Copy c'tor
ReplierOptions::ReplierOptions(const ReplierOptions &options)
{
    *this = options;
}

/// Move c'tor
ReplierOptions::ReplierOptions(ReplierOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ReplierOptions& ReplierOptions::operator=(const ReplierOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ReplierOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ReplierOptions& ReplierOptions::operator=(ReplierOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset class
void ReplierOptions::clear() noexcept
{
    pImpl = std::make_unique<ReplierOptionsImpl> ();
}

/// Destructor
ReplierOptions::~ReplierOptions() = default;

/// End point to bind to
void ReplierOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

std::string ReplierOptions::getAddress() const
{
    return pImpl->mOptions.getAddress();
}

bool ReplierOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// ZAP Options
void ReplierOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
}

UAuth::ZAPOptions ReplierOptions::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// High water mark
void ReplierOptions::setHighWaterMark(const int highWaterMark)
{
    pImpl->mOptions.setSendHighWaterMark(highWaterMark);
    pImpl->mOptions.setReceiveHighWaterMark(highWaterMark);
}

int ReplierOptions::getHighWaterMark() const noexcept
{
    return pImpl->mOptions.getSendHighWaterMark();
}

/// Reply options
UMPS::Messaging::RouterDealer::ReplyOptions
    ReplierOptions::getReplyOptions() const noexcept
{
    return pImpl->mOptions;
}

/*
/// Sets the callback
void ReplyOptions::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mCallback = callback;
    pImpl->mHaveCallback = true;
}

std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
    ReplyOptions::getCallback() const
{
    if (!haveCallback())
    {   
        throw std::runtime_error("Callback not set");
    }   
    return pImpl->mCallback;
}

bool ReplyOptions::haveCallback() const noexcept
{
    return pImpl->mHaveCallback;
}
*/
