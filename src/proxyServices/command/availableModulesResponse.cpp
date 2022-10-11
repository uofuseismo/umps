#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "private/services/moduleDetails.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::Command::AvailableModulesResponse"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::ProxyServices::Command;

namespace
{

void checkModuleDetails(const std::vector<ModuleDetails> &details)
{
    for (const auto &d : details)
    {   
        if (!d.haveName())
        {   
            throw std::invalid_argument("Module does not have name");
        }   
    }
}

nlohmann::json toJSONObject(const AvailableModulesResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["MessageVersion"] = response.getMessageVersion();
    obj["Identifier"] = response.getIdentifier();
    nlohmann::json moduleObjects;
    for (const auto &m : response)
    {
        moduleObjects.push_back(pack(m));
    }
    if (!moduleObjects.empty())
    {
        obj["Modules"] = moduleObjects;
    }
    else
    {
        obj["Modules"] = nullptr;
    }
    return obj;
}

AvailableModulesResponse objectToModules(const nlohmann::json &obj)
{
    AvailableModulesResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    response.setIdentifier(obj["Identifier"].get<int64_t> ());
    if (!obj["Modules"].is_null())
    {
        std::vector<ModuleDetails> modules;
        modules.reserve(obj["Modules"].size());
        for (const auto &moduleObject : obj["Modules"])
        {
            modules.push_back(unpack(moduleObject));
        }
        response.setModules(std::move(modules));
    }
    return response;
}

/// Create CBOR
std::string toCBORMessage(const AvailableModulesResponse &reqeuest)
{
    auto obj = toJSONObject(reqeuest);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

AvailableModulesResponse
    fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToModules(obj);
}

}

class AvailableModulesResponse::AvailableModulesResponseImpl
{
public:
    std::vector<ModuleDetails> mDetails;
    int64_t mIdentifier{0};
};

/// C'tor
AvailableModulesResponse::AvailableModulesResponse() :
    pImpl(std::make_unique<AvailableModulesResponseImpl> ())
{
}

/// Copy c'tor
AvailableModulesResponse::AvailableModulesResponse(
    const AvailableModulesResponse &response)
{
    *this = response;
}

/// Move c'tor
AvailableModulesResponse::AvailableModulesResponse(
    AvailableModulesResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
AvailableModulesResponse&
AvailableModulesResponse::operator=(const AvailableModulesResponse &reqeuest)
{
    if (&reqeuest == this){return *this;}
    pImpl = std::make_unique<AvailableModulesResponseImpl> (*reqeuest.pImpl);
    return *this;
}

/// Move assignment
AvailableModulesResponse& AvailableModulesResponse::operator=(
    AvailableModulesResponse &&reqeuest) noexcept
{
    if (&reqeuest == this){return *this;}
    pImpl = std::move(reqeuest.pImpl);
    return *this;
}

/// Reset class
void AvailableModulesResponse::clear() noexcept
{
    pImpl = std::make_unique<AvailableModulesResponseImpl> ();
}

/// Destructor
AvailableModulesResponse::~AvailableModulesResponse() = default;

/// Identiifer
void AvailableModulesResponse::setIdentifier(const int64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

int64_t AvailableModulesResponse::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

///  Convert message
std::string AvailableModulesResponse::toMessage() const
{
    return toCBORMessage(*this);
}

void AvailableModulesResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void AvailableModulesResponse::fromMessage(const char *messageIn,
                                           const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Set modules
void AvailableModulesResponse::setModules(std::vector<ModuleDetails> &&details)
{
    ::checkModuleDetails(details);
    pImpl->mDetails = std::move(details);
}

void AvailableModulesResponse::setModules(
    const std::vector<ModuleDetails> &details)
{
    ::checkModuleDetails(details);
    pImpl->mDetails = details;
}

std::vector<ModuleDetails> AvailableModulesResponse::getModules() const
{
    return pImpl->mDetails;
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableModulesResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableModulesResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableModulesResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableModulesResponse> ();
    return result;
}

/// Message type
std::string AvailableModulesResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string AvailableModulesResponse::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

/// Access
typename AvailableModulesResponse::ModuleType::iterator
    AvailableModulesResponse::begin()
{   
    return pImpl->mDetails.begin();
}

typename AvailableModulesResponse::ModuleType::iterator
    AvailableModulesResponse::end()
{
    return pImpl->mDetails.end();
}

typename AvailableModulesResponse::ModuleType::const_iterator
    AvailableModulesResponse::begin() const
{
    return pImpl->mDetails.begin();
}

typename AvailableModulesResponse::ModuleType::const_iterator
    AvailableModulesResponse::end() const
{
    return pImpl->mDetails.end();
}

typename AvailableModulesResponse::ModuleType::const_iterator
    AvailableModulesResponse::cbegin() const
{
    return pImpl->mDetails.cbegin();
}

typename AvailableModulesResponse::ModuleType::const_iterator
    AvailableModulesResponse::cend() const
{
    return pImpl->mDetails.cend();
}

ModuleDetails& AvailableModulesResponse::at(const size_t i)
{
    return pImpl->mDetails.at(i);
}

const ModuleDetails& AvailableModulesResponse::at(const size_t i) const
{
    return pImpl->mDetails.at(i);
}

ModuleDetails& AvailableModulesResponse::operator[](const size_t i)
{
    return pImpl->mDetails[i];
}

const ModuleDetails& AvailableModulesResponse::operator[](const size_t i) const
{
    return pImpl->mDetails[i];
}
