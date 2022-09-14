#include <map>
#include <string>
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RouterDealer;
namespace UAuth = UMPS::Authentication;

class RequestOptions::RequestOptionsImpl
{
public:
    UMPS::MessageFormats::Messages mMessageFormats;
    UAuth::ZAPOptions mZAPOptions;
    std::string mAddress;
    int mHighWaterMark = 0;
    bool mHaveCallback = false;
};

/// C'tor
RequestOptions::RequestOptions() :
    pImpl(std::make_unique<RequestOptionsImpl> ())
{
}

/// Copy c'tor
RequestOptions::RequestOptions(const RequestOptions &options)
{
    *this = options;
}

/// Move c'tor
RequestOptions::RequestOptions(RequestOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RequestOptions& RequestOptions::operator=(const RequestOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RequestOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RequestOptions& RequestOptions::operator=(RequestOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset class
void RequestOptions::clear() noexcept
{
    pImpl = std::make_unique<RequestOptionsImpl> ();
}

/// Destructor
RequestOptions::~RequestOptions() = default;

/// End point to bind to
void RequestOptions::setAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("End point is empty");
    }
    pImpl->mAddress = address;    
}

std::string RequestOptions::getAddress() const
{
   if (!haveAddress()){throw std::runtime_error("Address not set");}
   return pImpl->mAddress;
}

bool RequestOptions::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// ZAP Options
void RequestOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
}

UAuth::ZAPOptions RequestOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// High water mark
void RequestOptions::setHighWaterMark(const int highWaterMark)
{
    if (highWaterMark < 0)
    {
        throw std::invalid_argument("High water mark must be non-negative");
    }
    pImpl->mHighWaterMark = highWaterMark;
}

int RequestOptions::getHighWaterMark() const noexcept
{
    return pImpl->mHighWaterMark;
}

/*
/// Add a message subscription
void RequestOptions::addMessageFormat(
    std::unique_ptr<UMPS::MessageFormats::IMessage> &message)
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    if (pImpl->mMessageFormats.contains(message)){return;}
    pImpl->mMessageFormats.add(message);
}
*/

void RequestOptions::setMessageFormats(
    const UMPS::MessageFormats::Messages &messageFormats)
{
    if (messageFormats.empty())
    {
        throw std::invalid_argument("No message formats in container");
    }
    pImpl->mMessageFormats = messageFormats;
}

UMPS::MessageFormats::Messages RequestOptions::getMessageFormats() const
{
    if (!haveMessageFormats())
    {
        throw std::runtime_error("No message formats set");
    }
    return pImpl->mMessageFormats;
}

bool RequestOptions::haveMessageFormats() const noexcept
{
    return !pImpl->mMessageFormats.empty();
}
