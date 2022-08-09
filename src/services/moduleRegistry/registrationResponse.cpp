#include <string>
#include <nlohmann/json.hpp>
#include <boost/asio/ip/host_name.hpp>
#include "umps/services/moduleRegistry/registrationResponse.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ModuleRegistry;

#define MESSAGE_TYPE "UMPS::Services::ModuleRegistry::RegistrationResponse"

namespace
{

nlohmann::json toJSONObject(const RegistrationResponse &response)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    obj["MessageType"] = response.getMessageType();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode()); // Throws
    // Other stuff
    obj["Identifier"] = response.getIdentifier();
    return obj;
}

RegistrationResponse objectToResponse(const nlohmann::json &obj)
{
    RegistrationResponse response;
    // Essential stuff
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    response.setReturnCode(
        static_cast<RegistrationReturnCode> (obj["ReturnCode"].get<int> ()));
    // Optional stuff
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    return response;
}

RegistrationResponse fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

RegistrationResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

class RegistrationResponse::RegistrationResponseImpl
{
public:
    RegistrationReturnCode mReturnCode;
    uint64_t mIdentifier{0};
    bool mHaveReturnCode{false};
};

/// C'tor
RegistrationResponse::RegistrationResponse() :
    pImpl(std::make_unique<RegistrationResponseImpl> ())
{
}

/// Copy c'tor
RegistrationResponse::RegistrationResponse(const RegistrationResponse &response)
{
    *this = response;
}

/// Move c'tor
RegistrationResponse::RegistrationResponse(
    RegistrationResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
RegistrationResponse&
RegistrationResponse::operator=(const RegistrationResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<RegistrationResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
RegistrationResponse&
RegistrationResponse::operator=(RegistrationResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
RegistrationResponse::~RegistrationResponse() = default;

/// Clear
void RegistrationResponse::clear() noexcept
{
    pImpl = std::make_unique<RegistrationResponseImpl> ();
}

/// Return code
void RegistrationResponse::setReturnCode(
    const RegistrationReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
    pImpl->mHaveReturnCode = true;
}

RegistrationReturnCode RegistrationResponse::getReturnCode() const
{
    if (!haveReturnCode()){throw std::runtime_error("Return code not set");}
    return pImpl->mReturnCode;
}

bool RegistrationResponse::haveReturnCode() const noexcept
{
    return pImpl->mHaveReturnCode;
}

/// Identifier
void RegistrationResponse::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t RegistrationResponse::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string RegistrationResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string RegistrationResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void RegistrationResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void RegistrationResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void RegistrationResponse::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Convert message
std::string RegistrationResponse::toMessage() const
{
    return toCBOR();
}

void RegistrationResponse::fromMessage(const std::string &message)
{
   if (message.empty()){throw std::invalid_argument("Message is empty");}
   fromMessage(message.c_str(), message.size());
}

void RegistrationResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegistrationResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegistrationResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    RegistrationResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<RegistrationResponse> (); 
    return result;
}

/// Message type
std::string RegistrationResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}
