#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/moduleRegistry/registeredModulesResponse.hpp"
#include "umps/services/moduleRegistry/moduleDetails.hpp"
#include "private/services/packDetails.hpp"

using namespace UMPS::Services::ModuleRegistry;

#define MESSAGE_TYPE "UMPS::Services::ModuleRegistry::RegisteredModulesResponse"

namespace
{

nlohmann::json toJSONObject(const RegisteredModulesResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    auto modules = response.getModules();
    nlohmann::json modulesObject;
    for (const auto &details : modules)
    {
        auto detailsObject = pack(details);
        modulesObject.push_back(detailsObject);
    }
    obj["RegisteredModules"] = modulesObject;
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode()); // Throws
    obj["Identifier"] = response.getIdentifier();
    return obj;
}

RegisteredModulesResponse objectToResponse(const nlohmann::json &obj)
{
    RegisteredModulesResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    std::vector<ModuleDetails> modules;
    auto modulesObject = obj["RegisteredModules"];
    for (const auto &detailsObject : modulesObject)
    {
        auto details = unpack(detailsObject);
        modules.push_back(details);
    }
    if (!modules.empty()){response.setModules(modules);}
    response.setReturnCode(
        static_cast<RegisteredModulesReturnCode>
        (obj["ReturnCode"].get<int> ()));
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    return response;
}

RegisteredModulesResponse fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

RegisteredModulesResponse fromCBORMessage(const uint8_t *message,
                                         const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

class RegisteredModulesResponse::RegisteredModulesResponseImpl
{
public:
    std::vector<ModuleDetails> mModules;
    uint64_t mIdentifier{0};
    RegisteredModulesReturnCode
        mReturnCode{RegisteredModulesReturnCode::Success};
    bool mHaveReturnCode{false};
};


/// C'tor
RegisteredModulesResponse::RegisteredModulesResponse() :
    pImpl(std::make_unique<RegisteredModulesResponseImpl> ())
{
}

/// Copy c'tor
RegisteredModulesResponse::RegisteredModulesResponse(const RegisteredModulesResponse &response)
{
    *this = response;
}

/// Move c'tor
RegisteredModulesResponse::RegisteredModulesResponse(RegisteredModulesResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
RegisteredModulesResponse&
RegisteredModulesResponse::operator=(const RegisteredModulesResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<RegisteredModulesResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
RegisteredModulesResponse&
RegisteredModulesResponse::operator=(RegisteredModulesResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
RegisteredModulesResponse::~RegisteredModulesResponse() = default;

/// Clear
void RegisteredModulesResponse::clear() noexcept
{
    pImpl = std::make_unique<RegisteredModulesResponseImpl> ();
}

/// Identifier
void RegisteredModulesResponse::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t RegisteredModulesResponse::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
void RegisteredModulesResponse::setReturnCode(
    const RegisteredModulesReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
    pImpl->mHaveReturnCode = true;
}

RegisteredModulesReturnCode RegisteredModulesResponse::getReturnCode() const
{
    if (!haveReturnCode()){throw std::runtime_error("Return code not set");}
    return pImpl->mReturnCode;
}

bool RegisteredModulesResponse::haveReturnCode() const noexcept
{
    return pImpl->mHaveReturnCode;
}

/// Modules
void RegisteredModulesResponse::setModules(
    const std::vector<ModuleDetails> &modules)
{
    for (const auto &m : modules)
    {
        if (!m.haveName()){throw std::invalid_argument("Module name not set");}
    }
    pImpl->mModules = modules;
}

std::vector<ModuleDetails> RegisteredModulesResponse::getModules() const noexcept
{
    return pImpl->mModules;
}

/// Create JSON
std::string RegisteredModulesResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string RegisteredModulesResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void RegisteredModulesResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void RegisteredModulesResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void RegisteredModulesResponse::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Convert message
std::string RegisteredModulesResponse::toMessage() const
{
    return toCBOR();
}

void RegisteredModulesResponse::fromMessage(const std::string &message)
{
   if (message.empty()){throw std::invalid_argument("Message is empty");}
   fromMessage(message.c_str(), message.size());
}

void RegisteredModulesResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> RegisteredModulesResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegisteredModulesResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegisteredModulesResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegisteredModulesResponse> ();
    return result;
}

/// Message type
std::string RegisteredModulesResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}
