#include <nlohmann/json.hpp>
#include "umps/services/connectionInformation/availableBroadcastsRequest.hpp"

using namespace UMPS::Services::ConnectionInformation;

#define MESSAGE_TYPE "UMPS::Services::ConnectionInformation:AvailableBroadcastsRequest"

namespace
{

nlohmann::json toJSONObject(const AvailableBroadcastsRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    return obj;
}

AvailableBroadcastsRequest objectToRequest(const nlohmann::json &obj)
{
    AvailableBroadcastsRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {   
        throw std::invalid_argument("Message has invalid message type");
    }   
    return request;
}

AvailableBroadcastsRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

AvailableBroadcastsRequest fromCBORMessage(const uint8_t *message,
                                           const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

///--------------------------------------------------------------------------///
///                                 Implementation                           ///
///--------------------------------------------------------------------------///

class AvailableBroadcastsRequest::RequestImpl
{
public:

};

/// C'tor
AvailableBroadcastsRequest::AvailableBroadcastsRequest() :
    pImpl(std::make_unique<RequestImpl> ())
{
}

/// Copy c'tor
AvailableBroadcastsRequest::AvailableBroadcastsRequest(
    const AvailableBroadcastsRequest &request)
{
    *this = request;
}

/// Move c'tor 
AvailableBroadcastsRequest::AvailableBroadcastsRequest(
    AvailableBroadcastsRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment 
AvailableBroadcastsRequest& AvailableBroadcastsRequest::operator=(
    const AvailableBroadcastsRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<RequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
AvailableBroadcastsRequest& AvailableBroadcastsRequest::operator=(
    AvailableBroadcastsRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
AvailableBroadcastsRequest::~AvailableBroadcastsRequest() = default;

/// Message type
std::string AvailableBroadcastsRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Clone
std::unique_ptr<UMPS::MessageFormats::IMessage> 
    AvailableBroadcastsRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableBroadcastsRequest> (*this);
    return result;
}

/// Create instance
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableBroadcastsRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableBroadcastsRequest> (); 
    return result;
}

/// Convert message
std::string AvailableBroadcastsRequest::toMessage() const
{
    return toCBOR();
}

void AvailableBroadcastsRequest::fromMessage(const char *messageIn,
                                             const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// From CBOR
void AvailableBroadcastsRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void AvailableBroadcastsRequest::fromCBOR(const uint8_t *data,
                                          const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// From JSON
void AvailableBroadcastsRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// Create JSON
std::string AvailableBroadcastsRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string AvailableBroadcastsRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}
