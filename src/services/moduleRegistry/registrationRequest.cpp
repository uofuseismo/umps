#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/moduleRegistry/registrationRequest.hpp"
#include "umps/services/moduleRegistry/moduleDetails.hpp"

using namespace UMPS::Services::ModuleRegistry;

#define MESSAGE_TYPE "UMPS::Services::ModuleRegistry::RegistrationRequest"

namespace
{

nlohmann::json toJSONObject(const RegistrationRequest &request)
{
    nlohmann::json obj;
    auto details = request.getModuleDetails();;
    // Essential stuff (this will throw): 
    obj["MessageType"] = request.getMessageType();
    obj["ModuleName"] = details.getName(); // Throws
    // Other stuff
    obj["Executable"] = details.getExecutableName();
    obj["ProcessIdentifier"] = details.getProcessIdentifier();
    obj["ParentProcessIdentifier"] =  details.getParentProcessIdentifier();
    obj["Machine"] = details.getMachine();
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

RegistrationRequest objectToRequest(const nlohmann::json &obj)
{
    RegistrationRequest request;
    ModuleDetails details;
    // Essential stuff
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    details.setName(obj["ModuleName"].get<std::string> ());
    // Optional stuff
    details.setExecutableName(obj["Executable"].get<std::string> ());
    details.setProcessIdentifier(obj["ProcessIdentifier"].get<int64_t> ());
    details.setParentProcessIdentifier(
        obj["ParentProcessIdentifier"].get<int64_t> ());
    details.setMachine(obj["Machine"].get<std::string> ());
    request.setIdentifier(obj["Identifier"].get<uint64_t> ());

    request.setModuleDetails(details);
    return request;
}

RegistrationRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

RegistrationRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

class RegistrationRequest::RegistrationRequestImpl
{
public:
    ModuleDetails mModuleDetails;
    uint64_t mIdentifier{0};
    bool mHaveModuleDetails{false};
};


/// C'tor
RegistrationRequest::RegistrationRequest() :
    pImpl(std::make_unique<RegistrationRequestImpl> ())
{
}

/// Copy c'tor
RegistrationRequest::RegistrationRequest(const RegistrationRequest &request)
{
    *this = request;
}

/// Move c'tor
RegistrationRequest::RegistrationRequest(RegistrationRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
RegistrationRequest&
RegistrationRequest::operator=(const RegistrationRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<RegistrationRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
RegistrationRequest&
RegistrationRequest::operator=(RegistrationRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
RegistrationRequest::~RegistrationRequest() = default;

/// Clear
void RegistrationRequest::clear() noexcept
{
    pImpl = std::make_unique<RegistrationRequestImpl> ();
}

void RegistrationRequest::setModuleDetails(const ModuleDetails &details)
{
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    pImpl->mModuleDetails = details;
    pImpl->mHaveModuleDetails = true;
}

ModuleDetails RegistrationRequest::getModuleDetails() const
{
    if (!haveModuleDetails())
    {
        throw std::runtime_error("Module details not set");
    }
    return pImpl->mModuleDetails;
}

bool RegistrationRequest::haveModuleDetails() const noexcept
{
    return pImpl->mHaveModuleDetails;
}

/*
/// Module name
void RegistrationRequest::setModuleName(const std::string &module)
{
    if (isEmpty(module)){throw std::invalid_argument("Module is empty");}
    pImpl->mModuleName = module;
}

std::string RegistrationRequest::getModuleName() const
{
    if (!haveModuleName()){throw std::runtime_error("Module not set");}
    return pImpl->mModuleName;
}

bool RegistrationRequest::haveModuleName() const noexcept
{
    return !pImpl->mModuleName.empty();
}

/// Machine
void RegistrationRequest::setMachine(const std::string &machine) noexcept
{
    pImpl->mMachine = machine;
}

std::string RegistrationRequest::getMachine() const noexcept
{
    return pImpl->mMachine;
}
*/

/// Identifier
void RegistrationRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t RegistrationRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string RegistrationRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string RegistrationRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void RegistrationRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void RegistrationRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void RegistrationRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Convert message
std::string RegistrationRequest::toMessage() const
{
    return toCBOR();
}

void RegistrationRequest::fromMessage(const std::string &message)
{
   if (message.empty()){throw std::invalid_argument("Message is empty");}
   fromMessage(message.c_str(), message.size());
}

void RegistrationRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> RegistrationRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegistrationRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegistrationRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegistrationRequest> ();
    return result;
}

/// Message type
std::string RegistrationRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}
