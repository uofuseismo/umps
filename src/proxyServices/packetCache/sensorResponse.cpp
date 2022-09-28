#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <unordered_set>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/packetCache/sensorResponse.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::PacketCache::SensorResponse"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::ProxyServices::PacketCache;

namespace
{

nlohmann::json toJSONObject(const SensorResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["MessageVersion"] = response.getMessageVersion();
    auto messageNames = response.getNames();
    if (!messageNames.empty())
    {
        obj["Names"] = messageNames; 
    } 
    else
    {
        obj["Names"] = nullptr;
    }
    obj["Identifier"] = response.getIdentifier();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

SensorResponse objectToSensorResponse(const nlohmann::json &obj)
{
    SensorResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    if (!obj["Names"].is_null())
    {
        response.setNames(obj["Names"].get<std::unordered_set<std::string>> ());
    }
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    response.setReturnCode(static_cast<ReturnCode>
                           (obj["ReturnCode"].get<int> ()));
    return response;
}

SensorResponse fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToSensorResponse(obj);
}

SensorResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToSensorResponse(obj);
}

}

class SensorResponse::SensorResponseImpl
{
public:
    std::unordered_set<std::string> mNames;
    uint64_t mIdentifier = 0;
    ReturnCode mReturnCode = ReturnCode::SUCCESS;
};

/// C'tor
SensorResponse::SensorResponse() :
    pImpl(std::make_unique<SensorResponseImpl> ())
{
}

/// Copy assignment
SensorResponse::SensorResponse(const SensorResponse &response)
{
    *this = response;
}

/// Move assignment
SensorResponse::SensorResponse(SensorResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
SensorResponse& SensorResponse::operator=(const SensorResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<SensorResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
SensorResponse& SensorResponse::operator=(SensorResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Reset class
void SensorResponse::clear() noexcept
{
    pImpl = std::make_unique<SensorResponseImpl> ();
}

/// Destructor
SensorResponse::~SensorResponse() = default;

/// Message type
std::string SensorResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string SensorResponse::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

/// Identifier
void SensorResponse::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t SensorResponse::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
void SensorResponse::setReturnCode(const ReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
}

ReturnCode SensorResponse::getReturnCode() const noexcept
{
    return pImpl->mReturnCode;
}

/// Set names
void SensorResponse::setNames(
    const std::unordered_set<std::string> &names) noexcept
{
    pImpl->mNames = names;
}

std::unordered_set<std::string> SensorResponse::getNames() const noexcept
{
    return pImpl->mNames;
}

/// Create JSON
std::string SensorResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string SensorResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void SensorResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void SensorResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void SensorResponse::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string SensorResponse::toMessage() const
{
    return toCBOR();
}


void SensorResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void SensorResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> SensorResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<SensorResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    SensorResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<SensorResponse> (); 
    return result;
}

