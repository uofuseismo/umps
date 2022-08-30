#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/commandsResponse.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::CommandsResponse"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const CommandsResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["Commands"] = response.getCommands();
    return obj;
}

CommandsResponse objectToCommands(const nlohmann::json &obj)
{
    CommandsResponse response;
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
std::string toCBORMessage(const CommandsResponse &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

CommandsResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class CommandsResponse::CommandsResponseImpl
{
public:
    std::string mCommands;
};

/// C'tor
CommandsResponse::CommandsResponse() :
    pImpl(std::make_unique<CommandsResponseImpl> ())
{
}

/// Copy c'tor
CommandsResponse::CommandsResponse(const CommandsResponse &response)
{
    *this = response;
}

/// Move c'tor
CommandsResponse::CommandsResponse(CommandsResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
CommandsResponse& CommandsResponse::operator=(const CommandsResponse &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<CommandsResponseImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
CommandsResponse&
CommandsResponse::operator=(CommandsResponse &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void CommandsResponse::clear() noexcept
{
    pImpl = std::make_unique<CommandsResponseImpl> ();
}

/// Destructor
CommandsResponse::~CommandsResponse() = default;

/// Commands
void CommandsResponse::setCommands(const std::string &commands)
{
    pImpl->mCommands = commands;
}

std::string CommandsResponse::getCommands() const noexcept
{
    return pImpl->mCommands;
}

///  Convert message
std::string CommandsResponse::toMessage() const
{
    return toCBORMessage(*this);
}

void CommandsResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void CommandsResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> CommandsResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandsResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    CommandsResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandsResponse> ();
    return result;
}

/// Message type
std::string CommandsResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

