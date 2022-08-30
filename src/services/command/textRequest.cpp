#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/textRequest.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::TextRequest"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const TextRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["Command"] = request.getCommand(); 
    return obj;
}

TextRequest objectToCommands(const nlohmann::json &obj)
{
    TextRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setCommand(obj["Command"]);
    return request;
}

/// Create CBOR
std::string toCBORMessage(const TextRequest &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

TextRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class TextRequest::TextRequestImpl
{
public:
    std::string mCommand;
};

/// C'tor
TextRequest::TextRequest() :
    pImpl(std::make_unique<TextRequestImpl> ())
{
}

/// Copy c'tor
TextRequest::TextRequest(const TextRequest &request)
{
    *this = request;
}

/// Move c'tor
TextRequest::TextRequest(TextRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
TextRequest& TextRequest::operator=(const TextRequest &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<TextRequestImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
TextRequest& TextRequest::operator=(TextRequest &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void TextRequest::clear() noexcept
{
    pImpl = std::make_unique<TextRequestImpl> ();
}

/// Destructor
TextRequest::~TextRequest() = default;

///  Convert message
std::string TextRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void TextRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void TextRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> TextRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<TextRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    TextRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<TextRequest> ();
    return result;
}

/// Set command
void TextRequest::setCommand(const std::string &command)
{
    if (::isEmpty(command)){throw std::invalid_argument("Command is empty");}
    pImpl->mCommand = command;
}

std::string TextRequest::getCommand() const
{
    if (!haveCommand()){throw std::runtime_error("Command not set");}
    return pImpl->mCommand;
}

bool TextRequest::haveCommand() const noexcept
{
    return !pImpl->mCommand.empty();
}

/// Message type
std::string TextRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

