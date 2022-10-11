#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/command/availableModulesRequest.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::Command::AvailableModulesRequest"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::ProxyServices::Command;

namespace
{

nlohmann::json toJSONObject(const AvailableModulesRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["MessageVersion"] = request.getMessageVersion();
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

AvailableModulesRequest objectToModules(const nlohmann::json &obj)
{
    AvailableModulesRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setIdentifier(obj["Identifier"].get<int64_t> ());
    return request;
}

/// Create CBOR
std::string toCBORMessage(const AvailableModulesRequest &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

AvailableModulesRequest
    fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToModules(obj);
}

}

class AvailableModulesRequest::AvailableModulesRequestImpl
{
public:
    int64_t mIdentifier{0};
};

/// C'tor
AvailableModulesRequest::AvailableModulesRequest() :
    pImpl(std::make_unique<AvailableModulesRequestImpl> ())
{
}

/// Copy c'tor
AvailableModulesRequest::AvailableModulesRequest(
    const AvailableModulesRequest &request)
{
    *this = request;
}

/// Move c'tor
AvailableModulesRequest::AvailableModulesRequest(
    AvailableModulesRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
AvailableModulesRequest&
AvailableModulesRequest::operator=(const AvailableModulesRequest &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<AvailableModulesRequestImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
AvailableModulesRequest& AvailableModulesRequest::operator=(
    AvailableModulesRequest &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void AvailableModulesRequest::clear() noexcept
{
    pImpl = std::make_unique<AvailableModulesRequestImpl> ();
}

/// Destructor
AvailableModulesRequest::~AvailableModulesRequest() = default;

/// Identiifer
void AvailableModulesRequest::setIdentifier(const int64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

int64_t AvailableModulesRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

///  Convert message
std::string AvailableModulesRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void AvailableModulesRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void AvailableModulesRequest::fromMessage(const char *messageIn,
                                           const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableModulesRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableModulesRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableModulesRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableModulesRequest> ();
    return result;
}

/// Message type
std::string AvailableModulesRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string AvailableModulesRequest::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

