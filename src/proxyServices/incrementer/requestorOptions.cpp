#include <map>
#include <string>
#include <chrono>
#include "umps/proxyServices/incrementer/requestorOptions.hpp"
#include "umps/proxyServices/incrementer/itemsResponse.hpp"
#include "umps/proxyServices/incrementer/incrementResponse.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Incrementer;
namespace UAuth = UMPS::Authentication;

class RequestorOptions::RequestorOptionsImpl
{
public:
    RequestorOptionsImpl()
    {
        mOptions.setHighWaterMark(2048);
        std::unique_ptr<UMPS::MessageFormats::IMessage> incrementResponse
            = std::make_unique<IncrementResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> itemsResponse
            = std::make_unique<ItemsResponse> ();
        UMPS::MessageFormats::Messages messageFormats;
        messageFormats.add(incrementResponse);
        messageFormats.add(itemsResponse);
        mOptions.setMessageFormats(messageFormats);

    }
    UMPS::Messaging::RouterDealer::RequestOptions mOptions;
};

/// C'tor
RequestorOptions::RequestorOptions() :
    pImpl(std::make_unique<RequestorOptionsImpl> ())
{
}

/// Copy c'tor
RequestorOptions::RequestorOptions(const RequestorOptions &options)
{
    *this = options;
}

/// Move c'tor
RequestorOptions::RequestorOptions(RequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RequestorOptions& RequestorOptions::operator=(const RequestorOptions &options)
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

/// Reset class
void RequestorOptions::clear() noexcept
{
    pImpl = std::make_unique<RequestorOptionsImpl> ();
}

/// Destructor
RequestorOptions::~RequestorOptions() = default;

/// End point to bind to
void RequestorOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

std::string RequestorOptions::getAddress() const
{
   return pImpl->mOptions.getAddress();
}

bool RequestorOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// ZAP Options
void RequestorOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
}

UAuth::ZAPOptions RequestorOptions::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// High water mark
void RequestorOptions::setHighWaterMark(const int highWaterMark)
{
    pImpl->mOptions.setHighWaterMark(highWaterMark);
}

int RequestorOptions::getHighWaterMark() const noexcept
{
    return pImpl->mOptions.getHighWaterMark();
}

/// Request options
UMPS::Messaging::RouterDealer::RequestOptions
     RequestorOptions::getRequestOptions() const noexcept
{
    return pImpl->mOptions;
}
