#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/packetCache/sensorRequest.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::PacketCache::SensorRequest"

using namespace UMPS::ProxyServices::PacketCache;

namespace
{

nlohmann::json toJSONObject(const SensorRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

SensorRequest objectToSensorRequest(const nlohmann::json &obj)
{
    SensorRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setIdentifier(obj["Identifier"]);
    return request;
}

SensorRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToSensorRequest(obj);
}

SensorRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToSensorRequest(obj);
}

}

class SensorRequest::SensorRequestImpl
{
public:
    uint64_t mIdentifier = 0;
};

/// C'tor
SensorRequest::SensorRequest() :
    pImpl(std::make_unique<SensorRequestImpl> ())
{
}

/// Copy assignment
SensorRequest::SensorRequest(const SensorRequest &request)
{
    *this = request;
}

/// Move assignment
SensorRequest::SensorRequest(SensorRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
SensorRequest& SensorRequest::operator=(const SensorRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<SensorRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
SensorRequest& SensorRequest::operator=(SensorRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Reset class
void SensorRequest::clear() noexcept
{
    pImpl = std::make_unique<SensorRequestImpl> ();
}

/// Destructor
SensorRequest::~SensorRequest() = default;

/// Message type
std::string SensorRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Identifier
void SensorRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t SensorRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string SensorRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string SensorRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void SensorRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void SensorRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void SensorRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string SensorRequest::toMessage() const
{
    return toCBOR();
}

void SensorRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> SensorRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<SensorRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    SensorRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<SensorRequest> (); 
    return result;
}

