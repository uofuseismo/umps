#include <iostream>
#include <string>
#include <set>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/incrementer/itemsResponse.hpp"

#define MESSAGE_TYPE "UMPS::Services::Incrementer::ItemsResponse"

using namespace UMPS::Services::Incrementer;

namespace
{

nlohmann::json toJSONObject(const ItemsResponse &response)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    obj["MessageType"] = response.getMessageType();
    if (response.haveItems())
    {
        obj["Value"] = response.getItems();
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

ItemsResponse objectToResponse(const nlohmann::json &obj)
{
    ItemsResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Essential stuff
    if (!obj["Value"].is_null())
    {
        response.setItems(obj["Value"].get<std::set<std::string>> ());
    }
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    auto code = static_cast<ReturnCode> (obj["ReturnCode"].get<int> ());
    response.setReturnCode(code);
    return response;
}

ItemsResponse fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

ItemsResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

class ItemsResponse::ItemsResponseImpl
{
public:
    std::set<std::string> mItems;
    uint64_t mIdentifier = 0;
    ReturnCode mCode = ReturnCode::SUCCESS;
};

/// C'tor
ItemsResponse::ItemsResponse() :
    pImpl(std::make_unique<ItemsResponseImpl> ())
{
}

/// Copy c'tor
ItemsResponse::ItemsResponse(const ItemsResponse &response)
{
    *this = response;
}

/// Move c'tor
ItemsResponse::ItemsResponse(ItemsResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
ItemsResponse& ItemsResponse::operator=(const ItemsResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<ItemsResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
ItemsResponse&
    ItemsResponse::operator=(ItemsResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
ItemsResponse::~ItemsResponse() = default;

/// Clear
void ItemsResponse::clear() noexcept
{
    pImpl = std::make_unique<ItemsResponseImpl> ();
}

/// Value
void ItemsResponse::setItems(const std::set<std::string> &items)
{
    if (items.empty()){throw std::invalid_argument("items is empty");}
    pImpl->mItems = items;
}

std::set<std::string> ItemsResponse::getItems() const
{
    if (!haveItems()){throw std::runtime_error("Items not set");}
    return pImpl->mItems;
}

bool ItemsResponse::haveItems() const noexcept
{
    return !pImpl->mItems.empty();
}

/// Identifier
void ItemsResponse::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t ItemsResponse::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
void ItemsResponse::setReturnCode(const ReturnCode code) noexcept
{
    pImpl->mCode = code;
}

ReturnCode ItemsResponse::getReturnCode() const noexcept
{
    return pImpl->mCode;
}

/// Create JSON
std::string ItemsResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string ItemsResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void ItemsResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void ItemsResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void ItemsResponse::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string ItemsResponse::toMessage() const
{
    return toCBOR();
}

/// Convert from message
void ItemsResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> ItemsResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<ItemsResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    ItemsResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<ItemsResponse> ();
    return result;
}

/// Message type
std::string ItemsResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
} 
