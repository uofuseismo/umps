#include <iostream>
#include <string>
#include <sstream>
#include "umps/proxyServices/command/replierOptions.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UAuth = UMPS::Authentication;
namespace URouterDealer = UMPS::Messaging::RouterDealer;

class ReplierOptions::ReplierOptionsImpl
{
public:
    ReplierOptionsImpl()
    {
        std::ostringstream address;
        address << static_cast<void const *> (this);
        auto routingIdentifier = "module_" + address.str();
        mOptions.setRoutingIdentifier(routingIdentifier);
    }
    URouterDealer::ReplyOptions mOptions;
    ModuleDetails mDetails;
    bool mHaveDetails{false};
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
ReplierOptions::ReplierOptions(
    ReplierOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ReplierOptions&
ReplierOptions::operator=(const ReplierOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ReplierOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ReplierOptions&
ReplierOptions::operator=(ReplierOptions &&options) noexcept
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

/// Module details
void ReplierOptions::setModuleDetails(const ModuleDetails &details)
{
    if (!details.haveName())
    {
        throw std::invalid_argument("Module details name not set");
    }
    pImpl->mDetails = details;
    pImpl->mHaveDetails = true;
}

ModuleDetails ReplierOptions::getModuleDetails() const
{
    if (!haveModuleDetails())
    {
        throw std::runtime_error("Module details not set");
    }
    return pImpl->mDetails;
}

bool ReplierOptions::haveModuleDetails() const noexcept
{
    return pImpl->mHaveDetails;
}

/// Set address
void ReplierOptions::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

bool ReplierOptions::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// Callback
void ReplierOptions::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mOptions.setCallback(callback);
}

bool ReplierOptions::haveCallback() const noexcept
{
    return pImpl->mOptions.haveCallback();
}

/// ZAP options
void ReplierOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
}

/// HWM
void ReplierOptions::setHighWaterMark(const int hwm)
{
    pImpl->mOptions.setReceiveHighWaterMark(hwm);
    pImpl->mOptions.setSendHighWaterMark(hwm);
}

/// Options
URouterDealer::ReplyOptions ReplierOptions::getOptions() const
{
    if (!haveAddress()){throw std::runtime_error("Address not set");}
    if (!haveCallback()){throw std::runtime_error("Callback not set");}
    return pImpl->mOptions;
}
