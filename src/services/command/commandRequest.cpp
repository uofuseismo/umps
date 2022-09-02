#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/commandRequest.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::CommandRequest"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const CommandRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["Command"] = request.getCommand(); 
    return obj;
}

CommandRequest objectToCommands(const nlohmann::json &obj)
{
    CommandRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setCommand(obj["Command"]);
    return request;
}

/// Create CBOR
std::string toCBORMessage(const CommandRequest &request)
{
    auto obj = toJSONObject(request);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

CommandRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class CommandRequest::CommandRequestImpl
{
public:
    std::string mCommand;
};

/// C'tor
CommandRequest::CommandRequest() :
    pImpl(std::make_unique<CommandRequestImpl> ())
{
}

/// Copy c'tor
CommandRequest::CommandRequest(const CommandRequest &request)
{
    *this = request;
}

/// Move c'tor
CommandRequest::CommandRequest(CommandRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
CommandRequest& CommandRequest::operator=(const CommandRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<CommandRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
CommandRequest& CommandRequest::operator=(CommandRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Reset class
void CommandRequest::clear() noexcept
{
    pImpl = std::make_unique<CommandRequestImpl> ();
}

/// Destructor
CommandRequest::~CommandRequest() = default;

///  Convert message
std::string CommandRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void CommandRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void CommandRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> CommandRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    CommandRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandRequest> ();
    return result;
}

/// Set command
void CommandRequest::setCommand(const std::string &command)
{
    if (::isEmpty(command)){throw std::invalid_argument("Command is empty");}
    pImpl->mCommand = command;
}

std::string CommandRequest::getCommand() const
{
    if (!haveCommand()){throw std::runtime_error("Command not set");}
    return pImpl->mCommand;
}

bool CommandRequest::haveCommand() const noexcept
{
    return !pImpl->mCommand.empty();
}

/// Message type
std::string CommandRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

