#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/messageFormats/text.hpp"

#define MESSAGE_TYPE "UMPS::MessageFormats::Text"

using namespace UMPS::MessageFormats;

namespace
{

nlohmann::json toJSONObject(const Text &text)
{
    nlohmann::json obj;
    obj["MessageType"] = text.getMessageType();
    obj["Contents"] = text.getContents();
    return obj;
}

Text objectToText(const nlohmann::json &obj)
{
    Text text;
    if (obj["MessageType"] != text.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    text.setContents(obj["Contents"].get<std::string> ());
    return text;
}

/// Create CBOR
std::string toCBORMessage(const Text &text)
{
    auto obj = toJSONObject(text);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

Text fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToText(obj);
}

}

class Text::TextImpl
{
public:
    std::string mContents;
};

/// C'tor
Text::Text() :
    pImpl(std::make_unique<TextImpl> ())
{
}

/// Copy c'tor
Text::Text(const Text &text)
{
    *this = text;
}

/// Move c'tor
Text::Text(Text &&text) noexcept
{
    *this = std::move(text);
}

/// Copy assignment
Text& Text::operator=(const Text &text)
{
    if (&text == this){return *this;}
    pImpl = std::make_unique<TextImpl> (*text.pImpl);
    return *this;
}

/// Move assignment
Text& Text::operator=(Text &&text) noexcept
{
    if (&text == this){return *this;}
    pImpl = std::move(text.pImpl);
    return *this;
}

/// Destructor
Text::~Text() = default;

/// Reset the class 
void Text::clear() noexcept
{
    pImpl->mContents.clear();
}

/// Contents
void Text::setContents(const std::string &contents) noexcept
{
    pImpl->mContents = contents;
}

std::string Text::getContents() const noexcept
{
    return pImpl->mContents;
}

///  Convert message
std::string Text::toMessage() const
{
    return toCBORMessage(*this);
}

void Text::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> Text::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Text> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Text::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Text> (); 
    return result;
}

/// Message type
std::string Text::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

