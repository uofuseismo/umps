#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/availableCommandsResponse.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::AvailableCommandsResponse"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const AvailableCommandsResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["MessageVersion"] = response.getMessageVersion();
    obj["Commands"] = response.getCommands();
    return obj;
}

AvailableCommandsResponse objectToCommands(const nlohmann::json &obj)
{
    AvailableCommandsResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    if (!obj["Commands"].is_null())
    {
        response.setCommands(obj["Commands"]);
    }
    return response;
}

/// Create CBOR
std::string toCBORMessage(const AvailableCommandsResponse &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

AvailableCommandsResponse
    fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class AvailableCommandsResponse::AvailableCommandsResponseImpl
{
public:
    std::string mCommands;
};

/// C'tor
AvailableCommandsResponse::AvailableCommandsResponse() :
    pImpl(std::make_unique<AvailableCommandsResponseImpl> ())
{
}

/// Copy c'tor
AvailableCommandsResponse::AvailableCommandsResponse(
    const AvailableCommandsResponse &response)
{
    *this = response;
}

/// Move c'tor
AvailableCommandsResponse::AvailableCommandsResponse(
    AvailableCommandsResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
AvailableCommandsResponse&
AvailableCommandsResponse::operator=(const AvailableCommandsResponse &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<AvailableCommandsResponseImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
AvailableCommandsResponse&
AvailableCommandsResponse::operator=(
    AvailableCommandsResponse &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void AvailableCommandsResponse::clear() noexcept
{
    pImpl = std::make_unique<AvailableCommandsResponseImpl> ();
}

/// Destructor
AvailableCommandsResponse::~AvailableCommandsResponse() = default;

/// Commands
void AvailableCommandsResponse::setCommands(const std::string &commands)
{
    pImpl->mCommands = commands;
}

std::string AvailableCommandsResponse::getCommands() const noexcept
{
    return pImpl->mCommands;
}

///  Convert message
std::string AvailableCommandsResponse::toMessage() const
{
    return toCBORMessage(*this);
}

void AvailableCommandsResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void AvailableCommandsResponse::fromMessage(const char *messageIn,
                                            const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage>
AvailableCommandsResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableCommandsResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableCommandsResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableCommandsResponse> ();
    return result;
}

/// Message type
std::string AvailableCommandsResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string AvailableCommandsResponse::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

