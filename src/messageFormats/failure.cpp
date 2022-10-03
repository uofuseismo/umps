#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/messageFormats/failure.hpp"

#define MESSAGE_TYPE "UMPS::MessageFormats::Failure"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::MessageFormats;

namespace
{

nlohmann::json toJSONObject(const Failure &message)
{
    nlohmann::json obj;
    obj["MessageType"] = message.getMessageType();
    obj["MessageVersion"] = message.getMessageVersion();
    obj["Details"] = message.getDetails();
    return obj;
}

Failure objectToFailure(const nlohmann::json &obj)
{
    Failure message;
    if (obj["MessageType"] != message.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    message.setDetails(obj["Details"].get<std::string> ());
    return message;
}

/// Create CBOR
std::string toCBORMessage(const Failure &message)
{
    auto obj = toJSONObject(message);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

Failure fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToFailure(obj);
}

}

class Failure::FailureImpl
{
public:
    /// Details behind the failure
    std::string mDetails;
};

/// C'tor
Failure::Failure() :
    pImpl(std::make_unique<FailureImpl> ())
{
}

/// Copy c'tor
Failure::Failure(const Failure &message)
{
    *this = message;
}

/// Move c'tor
Failure::Failure(Failure &&message) noexcept
{
    *this = std::move(message);
}

/// Copy assignment
Failure& Failure::operator=(const Failure &message)
{
    if (&message == this){return *this;}
    pImpl = std::make_unique<FailureImpl> (*message.pImpl);
    return *this;
}

/// Move assignment
Failure& Failure::operator=(Failure &&message) noexcept
{
    if (&message == this){return *this;}
    pImpl = std::move(message.pImpl);
    return *this;
}

/// Destructor
Failure::~Failure() = default;

/// Reset the class 
void Failure::clear() noexcept
{
    pImpl->mDetails.clear();
}

/// Details
void Failure::setDetails(const std::string &details) noexcept
{
    pImpl->mDetails = details;
}

std::string Failure::getDetails() const noexcept
{
    return pImpl->mDetails;
}

///  Convert message
std::string Failure::toMessage() const
{
    return toCBORMessage(*this);
}

void Failure::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());   
}

void Failure::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> Failure::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Failure> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Failure::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Failure> (); 
    return result;
}

/// Message type
std::string Failure::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string Failure::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}
