#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/incrementer/response.hpp"

#define MESSAGE_TYPE "UMPS::Services::Incrementer::Response"

using namespace UMPS::Services::Incrementer;

namespace
{

nlohmann::json toJSONObject(const Response &response)
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

Response objectToResponse(const nlohmann::json &obj)
{
    Response response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Essential stuff
    if (!obj["Value"].is_null())
    {
        response.setValue(obj["Value"].get<uint64_t> ());
    }
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    auto code = static_cast<ReturnCode> (obj["ReturnCode"].get<int> ());
    response.setReturnCode(code);
    return response;
}

Response fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

Response fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

class Response::ResponseImpl
{
public:
    uint64_t mValue = 0;
    uint64_t mIdentifier = 0;
    ReturnCode mCode = ReturnCode::SUCCESS;
    bool mHaveValue = false;
};

/// C'tor
Response::Response() :
    pImpl(std::make_unique<ResponseImpl> ())
{
}

/// Copy c'tor
Response::Response(const Response &response)
{
    *this = response;
}

/// Move c'tor
Response::Response(Response &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
Response& Response::operator=(const Response &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<ResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
Response& Response::operator=(Response &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
Response::~Response() = default;

/// Clear
void Response::clear() noexcept
{
    pImpl = std::make_unique<ResponseImpl> ();
}

/// Value
void Response::setValue(const uint64_t value) noexcept
{
    pImpl->mValue = value;
    pImpl->mHaveValue = true;
}

uint64_t Response::getValue() const
{
    if (!haveValue()){throw std::runtime_error("Value not set");}
    return pImpl->mValue;
}

bool Response::haveValue() const noexcept
{
    return pImpl->mHaveValue;
}

/// Identifier
void Response::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t Response::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
void Response::setReturnCode(const ReturnCode code) noexcept
{
    pImpl->mCode = code;
}

ReturnCode Response::getReturnCode() const noexcept
{
    return pImpl->mCode;
}

/// Create JSON
std::string Response::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string Response::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void Response::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void Response::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void Response::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string Response::toMessage() const
{
    return toCBOR();
}

/// Convert from message
void Response::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> Response::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<Response> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Response::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<Response> ();
    return result;
}

/// Message type
std::string Response::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
} 
