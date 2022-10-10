#include <iostream>
#include <string>
#include <functional>
#include "umps/messaging/socketOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging;
namespace UAuth = UMPS::Authentication;

namespace
{
std::chrono::milliseconds
resolveTimeOut(const std::chrono::milliseconds &timeOut)
{
    constexpr std::chrono::milliseconds zero{0};
    std::chrono::milliseconds result{0};
    if (timeOut >= zero)
    {
        result = timeOut;
    }
    else
    {
        result = std::chrono::milliseconds{-1};
    }
    return result;
}
}

class SocketOptions::SocketOptionsImpl
{
public:
    UMPS::MessageFormats::Messages mMessageFormats;
    UAuth::ZAPOptions mZAPOptions;
    std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &, const void *, size_t)> mCallback;
    std::string mAddress;
    std::string mRoutingIdentifier;
    std::chrono::milliseconds mSendTimeOut{-1}; // Infinite
    std::chrono::milliseconds mReceiveTimeOut{-1}; // Infinite
    std::chrono::milliseconds mLingerPeriod{-1}; // Infinite
    int mSendHighWaterMark{0}; // Infinite
    int mReceiveHighWaterMark{0}; // Infinite
    bool mHaveCallback{false};
};

/// C'tor
SocketOptions::SocketOptions() :
    pImpl(std::make_unique<SocketOptionsImpl> ())
{
}

/// Copy c'tor
SocketOptions::SocketOptions(const SocketOptions &options)
{
    *this = options;
}

/// Move c'tor
SocketOptions::SocketOptions(SocketOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
SocketOptions& SocketOptions::operator=(const SocketOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<SocketOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
SocketOptions& SocketOptions::operator=(SocketOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset class
void SocketOptions::clear() noexcept
{
    pImpl = std::make_unique<SocketOptionsImpl> ();
}

/// Destructor
SocketOptions::~SocketOptions() = default;

/// Address
void SocketOptions::setAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    if (address.find("tcp://") != 0 && 
        address.find("ipc://") != 0 &&
        address.find("inproc://") != 0)
    {
        throw std::invalid_argument("Unhandled connection protocol: "
                                  + address);
    }
    pImpl->mAddress = address;
}

std::string SocketOptions::getAddress() const
{
    if (!haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    return pImpl->mAddress;
}

bool SocketOptions::haveAddress() const noexcept
{
    return !pImpl->mAddress.empty();
}

/// Timeout
void SocketOptions::setSendTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->mSendTimeOut = ::resolveTimeOut(timeOut);
}

std::chrono::milliseconds SocketOptions::getSendTimeOut() const noexcept
{
    return pImpl->mSendTimeOut;
}

void SocketOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->mReceiveTimeOut = ::resolveTimeOut(timeOut);
}

std::chrono::milliseconds SocketOptions::getReceiveTimeOut() const noexcept
{
    return pImpl->mReceiveTimeOut;
}

/// Linger period
void SocketOptions::setLingerPeriod(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->mLingerPeriod = ::resolveTimeOut(timeOut);
}

std::chrono::milliseconds SocketOptions::getLingerPeriod() const noexcept
{
    return pImpl->mLingerPeriod;
}


/// ZAP options
void SocketOptions::setZAPOptions(const UAuth::ZAPOptions &options) noexcept
{
    pImpl->mZAPOptions = options;
}

UAuth::ZAPOptions SocketOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// High water mark
void SocketOptions::setReceiveHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("High water mark must be positive");
    }
    pImpl->mReceiveHighWaterMark = hwm;
}

int SocketOptions::getReceiveHighWaterMark() const noexcept
{
    return pImpl->mReceiveHighWaterMark;
}

void SocketOptions::setSendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("High water mark must be positive");
    }
    pImpl->mSendHighWaterMark = hwm;
}

int SocketOptions::getSendHighWaterMark() const noexcept
{
    return pImpl->mSendHighWaterMark;
}

/// Sets the routing id
void SocketOptions::setRoutingIdentifier(const std::string &identifier)
{
    if (identifier.empty()){return;}
    constexpr size_t maxLength{255};
    if (identifier.size() > maxLength)
    {
        std::cerr << "Warning - truncating routing identifier to length 255"
                  << std::endl;
    }
    size_t stringLength = std::min(identifier.size(), maxLength);
    std::string temp{identifier, 0, stringLength};
    pImpl->mRoutingIdentifier = temp;
}

std::string SocketOptions::getRoutingIdentifier() const
{
    if (!haveRoutingIdentifier())
    {
        throw std::runtime_error("Routing identifier not set");
    }
    return pImpl->mRoutingIdentifier;
}

bool SocketOptions::haveRoutingIdentifier() const noexcept
{
    return !pImpl->mRoutingIdentifier.empty();
}

/// Sets the callback
void SocketOptions::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mCallback = callback;
    pImpl->mHaveCallback = true;
}

std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
    SocketOptions::getCallback() const
{
    if (!haveCallback())
    {
        throw std::runtime_error("Callback not set");
    }
    return pImpl->mCallback;
}

bool SocketOptions::haveCallback() const noexcept
{
    return pImpl->mHaveCallback;
}

/// Message formats
void SocketOptions::setMessageFormats(
    const UMPS::MessageFormats::Messages &messageFormats)
{
    if (messageFormats.empty())
    {
        throw std::invalid_argument("No message formats in container");
    }
    pImpl->mMessageFormats = messageFormats;
}

UMPS::MessageFormats::Messages SocketOptions::getMessageFormats() const
{
    if (!haveMessageFormats())
    {
        throw std::runtime_error("No message formats set");
    }
    return pImpl->mMessageFormats;
}

bool SocketOptions::haveMessageFormats() const noexcept
{
    return !pImpl->mMessageFormats.empty();
}
