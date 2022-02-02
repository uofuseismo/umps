#include <map>
#include <string>
#include <chrono>
#include "umps/proxyServices/packetCache/requestOptions.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UAuth = UMPS::Authentication;

class RequestOptions::RequestOptionsImpl
{
public:
    RequestOptionsImpl()
    {
        mOptions.setHighWaterMark(512);
        std::unique_ptr<UMPS::MessageFormats::IMessage> dataResponse
            = std::make_unique<DataResponse<double>> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> sensorResponse
            = std::make_unique<SensorResponse> ();
        UMPS::MessageFormats::Messages messageFormats;
        messageFormats.add(dataResponse);
        messageFormats.add(sensorResponse);
        mOptions.setMessageFormats(messageFormats);

    }
    UMPS::Messaging::RouterDealer::RequestOptions mOptions;
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
    pImpl->mOptions.setAddress(address);
}

std::string RequestOptions::getAddress() const
{
   return pImpl->mOptions.getAddress();
}

bool RequestOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// ZAP Options
void RequestOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
}

UAuth::ZAPOptions RequestOptions::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// High water mark
void RequestOptions::setHighWaterMark(const int highWaterMark)
{
    pImpl->mOptions.setHighWaterMark(highWaterMark);
}

int RequestOptions::getHighWaterMark() const noexcept
{
    return pImpl->mOptions.getHighWaterMark();
}

/// Request options
UMPS::Messaging::RouterDealer::RequestOptions
     RequestOptions::getRequestOptions() const noexcept
{
    return pImpl->mOptions;
}
