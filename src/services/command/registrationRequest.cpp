#include "umps/services/command/registrationRequest.hpp"
#include "umps/services/command/moduleDetails.hpp"
#include "private/services/moduleDetails.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::RegistrationRequest"
#define MESSAGE_VERSION "1.0.0"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const RegistrationRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["MessageVersion"] = request.getMessageVersion();
    obj["ModuleDetails"] = pack(request.getModuleDetails());
    return obj;
}

RegistrationRequest objectToRequest(const nlohmann::json &obj)
{
    RegistrationRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    request.setModuleDetails(unpack(obj["ModuleDetails"]));
    return request;
}

/// Create CBOR
std::string toCBORMessage(const RegistrationRequest &request)
{
    auto obj = toJSONObject(request);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
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

/// Reset class
void RegistrationRequest::clear() noexcept
{
    pImpl = std::make_unique<RegistrationRequestImpl> (); 
}

/// Destructor
RegistrationRequest::~RegistrationRequest() = default;

/// Module details
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

///  Convert message
std::string RegistrationRequest::toMessage() const
{
    return toCBORMessage(*this);
}

void RegistrationRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void RegistrationRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
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

/// Message version
std::string RegistrationRequest::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}
