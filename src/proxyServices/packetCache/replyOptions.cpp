#include <map>
#include <string>
#include <functional>
#include <chrono>
#include "umps/proxyServices/packetCache/replyOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UAuth = UMPS::Authentication;

class ReplyOptions::ReplyOptionsImpl
{
public:
    UMPS::Messaging::RouterDealer::ReplyOptions mOptions;
};

/// C'tor
ReplyOptions::ReplyOptions() :
    pImpl(std::make_unique<ReplyOptionsImpl> ())
{
}

/// Copy c'tor
ReplyOptions::ReplyOptions(const ReplyOptions &options)
{
    *this = options;
}

/// Move c'tor
ReplyOptions::ReplyOptions(ReplyOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ReplyOptions& ReplyOptions::operator=(const ReplyOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ReplyOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ReplyOptions& ReplyOptions::operator=(ReplyOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset class
void ReplyOptions::clear() noexcept
{
    pImpl = std::make_unique<ReplyOptionsImpl> ();
}

/// Destructor
ReplyOptions::~ReplyOptions() = default;

/// End point to bind to
void ReplyOptions::setEndPoint(const std::string &endPoint)
{
    pImpl->mOptions.setEndPoint(endPoint);
}

std::string ReplyOptions::getEndPoint() const
{
    return pImpl->mOptions.getEndPoint();
}

bool ReplyOptions::haveEndPoint() const noexcept
{
    return pImpl->mOptions.haveEndPoint();
}

/// ZAP Options
void ReplyOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
}

UAuth::ZAPOptions ReplyOptions::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// High water mark
void ReplyOptions::setHighWaterMark(const int highWaterMark)
{
    pImpl->mOptions.setHighWaterMark(highWaterMark);
}

int ReplyOptions::getHighWaterMark() const noexcept
{
    return pImpl->mOptions.getHighWaterMark();
}

/// Reply options
UMPS::Messaging::RouterDealer::ReplyOptions
    ReplyOptions::getReplyOptions() const noexcept
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
