#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/terminateRequest.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::TerminateRequest"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const TerminateRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["MessageVersion"] = request.getMessageVersion();
    return obj;
}

TerminateRequest objectToCommands(const nlohmann::json &obj)
{
    TerminateRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    return request;
}

/// Create CBOR
std::string toCBORMessage(const TerminateRequest &request)
{
    auto obj = toJSONObject(request);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

TerminateRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class TerminateRequest::TerminateRequestImpl
{
public:
};

/// C'tor
TerminateRequest::TerminateRequest() :
    pImpl(std::make_unique<TerminateRequestImpl> ())
{
}

/// Copy c'tor
TerminateRequest::TerminateRequest(const TerminateRequest &request)
{
    *this = request;
}

/// Move c'tor
TerminateRequest::TerminateRequest(TerminateRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
TerminateRequest& TerminateRequest::operator=(const TerminateRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<TerminateRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
TerminateRequest&
TerminateRequest::operator=(TerminateRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Reset class
void TerminateRequest::clear() noexcept
{
    pImpl = std::make_unique<TerminateRequestImpl> ();
}

/// Destructor
TerminateRequest::~TerminateRequest() = default;

///  Convert message
std::string TerminateRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void TerminateRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void TerminateRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> TerminateRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<TerminateRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    TerminateRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<TerminateRequest> ();
    return result;
}

/// Message type
std::string TerminateRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string TerminateRequest::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}
