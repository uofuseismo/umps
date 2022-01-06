#include <map>
#include <string>
#include <functional>
#include <chrono>
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UAuth = UMPS::Authentication;

class ReplyOptions::ReplyOptionsImpl
{
public:
    UMPS::MessageFormats::Messages mMessageFormats;
    UAuth::ZAPOptions mZAPOptions;
    std::function<
        std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *contents,
         const size_t length)
    > mCallback;
    std::string mEndPoint;
    int mHighWaterMark = 0;
    bool mHaveCallback = false;
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
    if (isEmpty(endPoint))
    {
        throw std::invalid_argument("End point is empty");
    }
    pImpl->mEndPoint = endPoint;    
}

std::string ReplyOptions::getEndPoint() const
{
   if (!haveEndPoint()){throw std::runtime_error("End point not set");}
   return pImpl->mEndPoint;
}

bool ReplyOptions::haveEndPoint() const noexcept
{
    return !pImpl->mEndPoint.empty();
}

/// ZAP Options
void ReplyOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
}

UAuth::ZAPOptions ReplyOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// High water mark
void ReplyOptions::setHighWaterMark(const int highWaterMark)
{
    if (highWaterMark < 0)
    {
        throw std::invalid_argument("High water mark must be non-negative");
    }
    pImpl->mHighWaterMark = highWaterMark;
}

int ReplyOptions::getHighWaterMark() const noexcept
{
    return pImpl->mHighWaterMark;
}

/// Add a message subscription
void ReplyOptions::addMessageFormat(
    std::unique_ptr<UMPS::MessageFormats::IMessage> &message)
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    if (pImpl->mMessageFormats.contains(message)){return;}
    pImpl->mMessageFormats.add(message);
}


UMPS::MessageFormats::Messages ReplyOptions::getMessageFormats() const noexcept
{
    return pImpl->mMessageFormats;
}

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
