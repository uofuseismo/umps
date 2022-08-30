#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/commandsRequest.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::CommandsRequest"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const CommandsRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    return obj;
}

CommandsRequest objectToCommands(const nlohmann::json &obj)
{
    CommandsRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    return request;
}

/// Create CBOR
std::string toCBORMessage(const CommandsRequest &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

CommandsRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class CommandsRequest::CommandsRequestImpl
{
public:
};

/// C'tor
CommandsRequest::CommandsRequest() :
    pImpl(std::make_unique<CommandsRequestImpl> ())
{
}

/// Copy c'tor
CommandsRequest::CommandsRequest(const CommandsRequest &request)
{
    *this = request;
}

/// Move c'tor
CommandsRequest::CommandsRequest(CommandsRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
CommandsRequest& CommandsRequest::operator=(const CommandsRequest &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<CommandsRequestImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
CommandsRequest& CommandsRequest::operator=(CommandsRequest &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void CommandsRequest::clear() noexcept
{
    pImpl = std::make_unique<CommandsRequestImpl> ();
}

/// Destructor
CommandsRequest::~CommandsRequest() = default;

///  Convert message
std::string CommandsRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void CommandsRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void CommandsRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> CommandsRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandsRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    CommandsRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandsRequest> ();
    return result;
}

/// Message type
std::string CommandsRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

