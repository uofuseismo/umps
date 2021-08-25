#include <iostream>
#include <string>
#include <limits>
#include <cassert>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "urts/modules/incrementer/request.hpp"
#include "private/applications/packetCache.hpp"

#define MESSAGE_TYPE "URTS::Modules::Incrementer::Request"

using namespace URTS::Modules::Incrementer;

namespace
{

nlohmann::json toJSONObject(const Request &request)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    obj["MessageType"] = request.getMessageType();
    obj["Item"] = request.getItem(); // Throws
    // Other stuff
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

Request objectToRequest(const nlohmann::json obj)
{
    Request request;
    // Essential stuff
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setItem(obj["Item"].get<std::string> ());
    // Optional stuff
    request.setIdentifier(obj["Identifier"].get<uint64_t> ());
    return request;
}

Request fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

Request fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

class Request::RequestImpl
{
public:
    std::string mItem;
    uint64_t mIdentifier = 0;
};

/// C'tor
Request::Request() :
    pImpl(std::make_unique<RequestImpl> ())
{
}

/// Copy c'tor
Request::Request(const Request &request)
{
    *this = request;
}

/// Move c'tor
Request::Request(Request &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
Request& Request::operator=(const Request &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<RequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
Request& Request::operator=(Request &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
Request::~Request() = default;

/// Clear
void Request::clear() noexcept
{
    pImpl = std::make_unique<RequestImpl> ();
}

/// Item 
void Request::setItem(const std::string &item)
{
    if (isEmpty(item)){throw std::invalid_argument("Item is empty");}
    pImpl->mItem = item;
}

void Request::setItem(const Item item) noexcept
{
   if (item == Item::PHASE_PICK)
   {
       setItem("PhasePick");
   }
   else if (item == Item::PHASE_ARRIVAL)
   {
       setItem("PhaseArrival");
   }
   else if (item == Item::EVENT)
   {
       setItem("Event");
   }
   else if (item == Item::ORIGIN)
   {
       setItem("Origin");
   }
   else
   {
#ifndef NDEBUG
       assert(false);
#else
       throw std::invalid_argument("Unhandled item");
#endif
   }
}

std::string Request::getItem() const
{
    if (!haveItem()){throw std::runtime_error("Item not set");}
    return pImpl->mItem;
}

bool Request::haveItem() const noexcept
{
    return !(pImpl->mItem.empty());
}

/// Identifier
void Request::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t Request::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string Request::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string Request::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void Request::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void Request::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void Request::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Copy this class
std::unique_ptr<URTS::MessageFormats::IMessage> Request::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<Request> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<URTS::MessageFormats::IMessage>
    Request::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<Request> ();
    return result;
}

/// Message type
std::string Request::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
} 
