#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/moduleRegistry/registeredModulesRequest.hpp"

using namespace UMPS::Services::ModuleRegistry;

#define MESSAGE_TYPE "UMPS::Services::ModuleRegistry::RegisteredModulesRequest"

namespace
{

nlohmann::json toJSONObject(const RegisteredModulesRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

RegisteredModulesRequest objectToRequest(const nlohmann::json &obj)
{
    RegisteredModulesRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setIdentifier(obj["Identifier"].get<uint64_t> ());
    return request;
}

RegisteredModulesRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

RegisteredModulesRequest fromCBORMessage(const uint8_t *message,
                                         const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

class RegisteredModulesRequest::RegisteredModulesRequestImpl
{
public:
    uint64_t mIdentifier{0};
};


/// C'tor
RegisteredModulesRequest::RegisteredModulesRequest() :
    pImpl(std::make_unique<RegisteredModulesRequestImpl> ())
{
}

/// Copy c'tor
RegisteredModulesRequest::RegisteredModulesRequest(const RegisteredModulesRequest &request)
{
    *this = request;
}

/// Move c'tor
RegisteredModulesRequest::RegisteredModulesRequest(RegisteredModulesRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
RegisteredModulesRequest&
RegisteredModulesRequest::operator=(const RegisteredModulesRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<RegisteredModulesRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
RegisteredModulesRequest&
RegisteredModulesRequest::operator=(RegisteredModulesRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
RegisteredModulesRequest::~RegisteredModulesRequest() = default;

/// Clear
void RegisteredModulesRequest::clear() noexcept
{
    pImpl = std::make_unique<RegisteredModulesRequestImpl> ();
}

/// Identifier
void RegisteredModulesRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t RegisteredModulesRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string RegisteredModulesRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string RegisteredModulesRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void RegisteredModulesRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void RegisteredModulesRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void RegisteredModulesRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Convert message
std::string RegisteredModulesRequest::toMessage() const
{
    return toCBOR();
}

void RegisteredModulesRequest::fromMessage(const std::string &message)
{
   if (message.empty()){throw std::invalid_argument("Message is empty");}
   fromMessage(message.c_str(), message.size());
}

void RegisteredModulesRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> RegisteredModulesRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegisteredModulesRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegisteredModulesRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegisteredModulesRequest> ();
    return result;
}

/// Message type
std::string RegisteredModulesRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}
