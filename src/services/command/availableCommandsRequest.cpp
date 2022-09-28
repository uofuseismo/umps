#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/availableCommandsRequest.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::AvailableCommandsRequest"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const AvailableCommandsRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["MessageVersion"] = request.getMessageVersion();
    return obj;
}

AvailableCommandsRequest objectToCommands(const nlohmann::json &obj)
{
    AvailableCommandsRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    return request;
}

/// Create CBOR
std::string toCBORMessage(const AvailableCommandsRequest &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

AvailableCommandsRequest
    fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class AvailableCommandsRequest::AvailableCommandsRequestImpl
{
public:
};

/// C'tor
AvailableCommandsRequest::AvailableCommandsRequest() :
    pImpl(std::make_unique<AvailableCommandsRequestImpl> ())
{
}

/// Copy c'tor
AvailableCommandsRequest::AvailableCommandsRequest(
    const AvailableCommandsRequest &request)
{
    *this = request;
}

/// Move c'tor
AvailableCommandsRequest::AvailableCommandsRequest(
    AvailableCommandsRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
AvailableCommandsRequest&
AvailableCommandsRequest::operator=(const AvailableCommandsRequest &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<AvailableCommandsRequestImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
AvailableCommandsRequest& AvailableCommandsRequest::operator=(
    AvailableCommandsRequest &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void AvailableCommandsRequest::clear() noexcept
{
    pImpl = std::make_unique<AvailableCommandsRequestImpl> ();
}

/// Destructor
AvailableCommandsRequest::~AvailableCommandsRequest() = default;

///  Convert message
std::string AvailableCommandsRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void AvailableCommandsRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void AvailableCommandsRequest::fromMessage(const char *messageIn,
                                           const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableCommandsRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableCommandsRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableCommandsRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableCommandsRequest> ();
    return result;
}

/// Message type
std::string AvailableCommandsRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string AvailableCommandsRequest::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

