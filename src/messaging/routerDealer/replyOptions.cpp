#include <iostream>
#include <map>
#include <cmath>
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
    std::string mAddress;
    std::string mRoutingIdentifier;
    std::chrono::milliseconds mSendTimeOut{-1};
    std::chrono::milliseconds mReceiveTimeOut{-1};
    std::chrono::milliseconds mPollingTimeOut{10};
    int mSendHighWaterMark{0}; // Infinite
    int mReceiveHighWaterMark{0}; // Infinite
    bool mHaveCallback{false};
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
void ReplyOptions::setAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("Address is empty");
    }
    pImpl->mAddress = address;
}

std::string ReplyOptions::getAddress() const
{
   if (!haveAddress()){throw std::runtime_error("Address not set");}
   return pImpl->mAddress;
}

bool ReplyOptions::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
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
void ReplyOptions::setSendHighWaterMark(const int highWaterMark)
{
    if (highWaterMark < 0)
    {
        throw std::invalid_argument("High water mark must be non-negative");
    }
    pImpl->mSendHighWaterMark = highWaterMark;
}

int ReplyOptions::getSendHighWaterMark() const noexcept
{
    return pImpl->mSendHighWaterMark;
}

void ReplyOptions::setReceiveHighWaterMark(const int highWaterMark)
{
    if (highWaterMark < 0)
    {
        throw std::invalid_argument("High water mark must be non-negative");
    }
    pImpl->mReceiveHighWaterMark = highWaterMark;
}

int ReplyOptions::getReceiveHighWaterMark() const noexcept
{
    return pImpl->mReceiveHighWaterMark;
}

/// Sets the routing id
void ReplyOptions::setRoutingIdentifier(const std::string &identifier)
{
    if (identifier.empty()){return;}
    constexpr size_t maxLength{255};
    size_t stringLength = std::min(identifier.size(), maxLength);
    std::string temp{identifier, 0, stringLength};
    pImpl->mRoutingIdentifier = temp;
}

std::string ReplyOptions::getRoutingIdentifier() const
{
    if (!haveRoutingIdentifier())
    {
        throw std::runtime_error("Routing identifier not set");
    }
    return pImpl->mRoutingIdentifier;
}

bool ReplyOptions::haveRoutingIdentifier() const noexcept
{
    return !pImpl->mRoutingIdentifier.empty();
}

/*
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
*/

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

/// Polling time out
void ReplyOptions::setPollingTimeOut(const std::chrono::milliseconds &timeOut)
{
    if (timeOut.count() < 0)
    {
        throw std::invalid_argument("Time out cannot be negative");
    }
    pImpl->mPollingTimeOut = timeOut;
}

std::chrono::milliseconds ReplyOptions::getPollingTimeOut() const noexcept
{
    return pImpl->mPollingTimeOut;
}
