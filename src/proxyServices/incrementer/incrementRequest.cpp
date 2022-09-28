#include <iostream>
#include <string>
#include <limits>
#include <cassert>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/incrementer/incrementRequest.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::Incrementer::IncrementRequest"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::ProxyServices::Incrementer;

namespace
{

nlohmann::json toJSONObject(const IncrementRequest &request)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    obj["MessageType"] = request.getMessageType();
    obj["MessageVersion"] = request.getMessageVersion();
    obj["Item"] = request.getItem(); // Throws
    // Other stuff
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

IncrementRequest objectToRequest(const nlohmann::json &obj)
{
    IncrementRequest request;
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

IncrementRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

IncrementRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

class IncrementRequest::IncrementRequestImpl
{
public:
    std::string mItem;
    uint64_t mIdentifier = 0;
};

/// C'tor
IncrementRequest::IncrementRequest() :
    pImpl(std::make_unique<IncrementRequestImpl> ())
{
}

/// Copy c'tor
IncrementRequest::IncrementRequest(const IncrementRequest &request)
{
    *this = request;
}

/// Move c'tor
IncrementRequest::IncrementRequest(IncrementRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
IncrementRequest& IncrementRequest::operator=(const IncrementRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<IncrementRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
IncrementRequest& IncrementRequest::operator=(IncrementRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
IncrementRequest::~IncrementRequest() = default;

/// Clear
void IncrementRequest::clear() noexcept
{
    pImpl = std::make_unique<IncrementRequestImpl> ();
}

/// Item 
void IncrementRequest::setItem(const std::string &item)
{
    if (isEmpty(item)){throw std::invalid_argument("Item is empty");}
    pImpl->mItem = item;
}

void IncrementRequest::setItem(const Item item) noexcept
{
   if (item == Item::AMPLITUDE)
   {
       setItem("Amplitude");
   }
   else if (item == Item::EVENT)
   {
       setItem("Event");
   }
   else if (item == Item::MAGNITUDE)
   {
       setItem("Magnitude");
   }
   else if (item == Item::ORIGIN)
   {
       setItem("Origin");
   }
   else if (item == Item::PHASE_PICK)
   {
       setItem("PhasePick");
   }
   else if (item == Item::PHASE_ARRIVAL)
   {
       setItem("PhaseArrival");
   }
   else
   {
#ifndef NDEBUG
       assert(false);
#else
       std::cerr << "Unhandled item" << std::endl;
#endif
   }
}

std::string IncrementRequest::getItem() const
{
    if (!haveItem()){throw std::runtime_error("Item not set");}
    return pImpl->mItem;
}

bool IncrementRequest::haveItem() const noexcept
{
    return !(pImpl->mItem.empty());
}

/// Identifier
void IncrementRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t IncrementRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string IncrementRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string IncrementRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void IncrementRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void IncrementRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void IncrementRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string IncrementRequest::toMessage() const
{
    return toCBOR();
}

void IncrementRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void IncrementRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> IncrementRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<IncrementRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    IncrementRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<IncrementRequest> ();
    return result;
}

/// Message type
std::string IncrementRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string IncrementRequest::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

