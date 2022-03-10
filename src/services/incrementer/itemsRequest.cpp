#include <iostream>
#include <string>
#include <limits>
#include <cassert>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/incrementer/itemsRequest.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Services::Incrementer::ItemsRequest"

using namespace UMPS::Services::Incrementer;

namespace
{

nlohmann::json toJSONObject(const ItemsRequest &request)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    obj["MessageType"] = request.getMessageType();
    // Other stuff
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

ItemsRequest objectToRequest(const nlohmann::json &obj)
{
    ItemsRequest request;
    // Essential stuff
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Optional stuff
    request.setIdentifier(obj["Identifier"].get<uint64_t> ());
    return request;
}

ItemsRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

ItemsRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

class ItemsRequest::ItemsRequestImpl
{
public:
    std::string mItem;
    uint64_t mIdentifier = 0;
};

/// C'tor
ItemsRequest::ItemsRequest() :
    pImpl(std::make_unique<ItemsRequestImpl> ())
{
}

/// Copy c'tor
ItemsRequest::ItemsRequest(const ItemsRequest &request)
{
    *this = request;
}

/// Move c'tor
ItemsRequest::ItemsRequest(ItemsRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
ItemsRequest& ItemsRequest::operator=(const ItemsRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<ItemsRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
ItemsRequest& ItemsRequest::operator=(ItemsRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
ItemsRequest::~ItemsRequest() = default;

/// Clear
void ItemsRequest::clear() noexcept
{
    pImpl = std::make_unique<ItemsRequestImpl> ();
}

/// Identifier
void ItemsRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t ItemsRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string ItemsRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string ItemsRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void ItemsRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void ItemsRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void ItemsRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string ItemsRequest::toMessage() const
{
    return toCBOR();
}

void ItemsRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> ItemsRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<ItemsRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    ItemsRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<ItemsRequest> ();
    return result;
}

/// Message type
std::string ItemsRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}
