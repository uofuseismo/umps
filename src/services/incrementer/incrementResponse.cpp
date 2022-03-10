#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/incrementer/incrementResponse.hpp"

#define MESSAGE_TYPE "UMPS::Services::Incrementer::IncrementResponse"

using namespace UMPS::Services::Incrementer;

namespace
{

nlohmann::json toJSONObject(const IncrementResponse &response)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    obj["MessageType"] = response.getMessageType();
    if (response.haveValue())
    {
        obj["Value"] = response.getValue();
    }
    else
    {
        obj["Value"] = nullptr;
    } 
    // Other stuff
    obj["Identifier"] = response.getIdentifier();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

IncrementResponse objectToResponse(const nlohmann::json &obj)
{
    IncrementResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Essential stuff
    if (!obj["Value"].is_null())
    {
        response.setValue(obj["Value"].get<int64_t> ());
    }
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    auto code = static_cast<ReturnCode> (obj["ReturnCode"].get<int> ());
    response.setReturnCode(code);
    return response;
}

IncrementResponse fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

IncrementResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

class IncrementResponse::IncrementResponseImpl
{
public:
    int64_t mValue = 0;
    uint64_t mIdentifier = 0;
    ReturnCode mCode = ReturnCode::SUCCESS;
    bool mHaveValue = false;
};

/// C'tor
IncrementResponse::IncrementResponse() :
    pImpl(std::make_unique<IncrementResponseImpl> ())
{
}

/// Copy c'tor
IncrementResponse::IncrementResponse(const IncrementResponse &response)
{
    *this = response;
}

/// Move c'tor
IncrementResponse::IncrementResponse(IncrementResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
IncrementResponse&
    IncrementResponse::operator=(const IncrementResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<IncrementResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
IncrementResponse&
    IncrementResponse::operator=(IncrementResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
IncrementResponse::~IncrementResponse() = default;

/// Clear
void IncrementResponse::clear() noexcept
{
    pImpl = std::make_unique<IncrementResponseImpl> ();
}

/// Value
void IncrementResponse::setValue(const int64_t value) noexcept
{
    pImpl->mValue = value;
    pImpl->mHaveValue = true;
}

int64_t IncrementResponse::getValue() const
{
    if (!haveValue()){throw std::runtime_error("Value not set");}
    return pImpl->mValue;
}

bool IncrementResponse::haveValue() const noexcept
{
    return pImpl->mHaveValue;
}

/// Identifier
void IncrementResponse::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t IncrementResponse::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
void IncrementResponse::setReturnCode(const ReturnCode code) noexcept
{
    pImpl->mCode = code;
}

ReturnCode IncrementResponse::getReturnCode() const noexcept
{
    return pImpl->mCode;
}

/// Create JSON
std::string IncrementResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string IncrementResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void IncrementResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void IncrementResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void IncrementResponse::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string IncrementResponse::toMessage() const
{
    return toCBOR();
}

/// Convert from message
void IncrementResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> IncrementResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<IncrementResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    IncrementResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<IncrementResponse> ();
    return result;
}

/// Message type
std::string IncrementResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
} 
