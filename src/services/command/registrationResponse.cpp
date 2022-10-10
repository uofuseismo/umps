#include "umps/services/command/registrationResponse.hpp"
#include "umps/services/command/moduleDetails.hpp"
#include <nlohmann/json.hpp>

#define MESSAGE_TYPE "UMPS::Services::Command::RegistrationResponse"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const RegistrationResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["MessageVersion"] = response.getMessageVersion();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

RegistrationResponse objectToResponse(const nlohmann::json &obj)
{
    RegistrationResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    response.setReturnCode(
        static_cast<RegistrationReturnCode> (obj["ReturnCode"].get<int> ()));
    return response;
}

/// Create CBOR
std::string toCBORMessage(const RegistrationResponse &response)
{
    auto obj = toJSONObject(response);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}   

RegistrationResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

class RegistrationResponse::RegistrationResponseImpl
{
public:
    RegistrationReturnCode mReturnCode;
    bool mHaveReturnCode{false};
};

/// C'tor
RegistrationResponse::RegistrationResponse() :
    pImpl(std::make_unique<RegistrationResponseImpl> ())
{
}

/// Copy c'tor
RegistrationResponse::RegistrationResponse(const RegistrationResponse &response)
{
    *this = response;
}

/// Move c'tor
RegistrationResponse::RegistrationResponse(
    RegistrationResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
RegistrationResponse&
RegistrationResponse::operator=(const RegistrationResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<RegistrationResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
RegistrationResponse&
RegistrationResponse::operator=(RegistrationResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Reset class
void RegistrationResponse::clear() noexcept
{
    pImpl = std::make_unique<RegistrationResponseImpl> (); 
}

/// Destructor
RegistrationResponse::~RegistrationResponse() = default;

/// Module details
void RegistrationResponse::setReturnCode(
    const RegistrationReturnCode returnCode) noexcept
{
    pImpl->mReturnCode = returnCode;
    pImpl->mHaveReturnCode = true;
}

RegistrationReturnCode RegistrationResponse::getReturnCode() const
{
    if (!haveReturnCode())
    {
        throw std::runtime_error("Return code not set");
    }
    return pImpl->mReturnCode;
}

bool RegistrationResponse::haveReturnCode() const noexcept
{
    return pImpl->mHaveReturnCode;
}

///  Convert message
std::string RegistrationResponse::toMessage() const
{
    return toCBORMessage(*this);
}

void RegistrationResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void RegistrationResponse::fromMessage(
    const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegistrationResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegistrationResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegistrationResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegistrationResponse> (); 
    return result;
}

/// Message type
std::string RegistrationResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string RegistrationResponse::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}
