#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/terminateResponse.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::TerminateResponse"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const TerminateResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

TerminateResponse objectToTerminates(const nlohmann::json &obj)
{
    TerminateResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    response.setReturnCode(static_cast<TerminateReturnCode> (obj["ReturnCode"]));
    return response;
}

/// Create CBOR
std::string toCBORMessage(const TerminateResponse &response)
{
    auto obj = toJSONObject(response);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

TerminateResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToTerminates(obj);
}

}

class TerminateResponse::TerminateResponseImpl
{
public:
    std::string mResponse;
    TerminateReturnCode mReturnCode;
    bool mHaveResponse{false};
    bool mHaveReturnCode{false};
};

/// C'tor
TerminateResponse::TerminateResponse() :
    pImpl(std::make_unique<TerminateResponseImpl> ())
{
}

/// Copy c'tor
TerminateResponse::TerminateResponse(const TerminateResponse &response)
{
    *this = response;
}

/// Move c'tor
TerminateResponse::TerminateResponse(TerminateResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
TerminateResponse& TerminateResponse::operator=(const TerminateResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<TerminateResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
TerminateResponse& TerminateResponse::operator=(TerminateResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Reset class
void TerminateResponse::clear() noexcept
{
    pImpl = std::make_unique<TerminateResponseImpl> ();
}

/// Destructor
TerminateResponse::~TerminateResponse() = default;

///  Convert message
std::string TerminateResponse::toMessage() const
{
    return toCBORMessage(*this);
}

void TerminateResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void TerminateResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> TerminateResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<TerminateResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    TerminateResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<TerminateResponse> ();
    return result;
}

/// Return code 
void TerminateResponse::setReturnCode(const TerminateReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
    pImpl->mHaveReturnCode = true;
}

TerminateReturnCode TerminateResponse::getReturnCode() const
{
    if (!haveReturnCode()){throw std::runtime_error("Return code not set");}
    return pImpl->mReturnCode;
}

bool TerminateResponse::haveReturnCode() const noexcept
{
    return pImpl->mHaveReturnCode;
}


/// Message type
std::string TerminateResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

