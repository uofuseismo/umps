#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/command/commandResponse.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Services::Command::CommandResponse"

using namespace UMPS::Services::Command;

namespace
{

nlohmann::json toJSONObject(const CommandResponse &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    obj["Response"] = response.getResponse();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

CommandResponse objectToCommands(const nlohmann::json &obj)
{
    CommandResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    response.setResponse(obj["Response"]);
    response.setReturnCode(static_cast<CommandReturnCode> (obj["ReturnCode"]));
    return response;
}

/// Create CBOR
std::string toCBORMessage(const CommandResponse &response)
{
    auto obj = toJSONObject(response);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

CommandResponse fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToCommands(obj);
}

}

class CommandResponse::CommandResponseImpl
{
public:
    std::string mResponse;
    CommandReturnCode mReturnCode;
    bool mHaveResponse{false};
    bool mHaveReturnCode{false};
};

/// C'tor
CommandResponse::CommandResponse() :
    pImpl(std::make_unique<CommandResponseImpl> ())
{
}

/// Copy c'tor
CommandResponse::CommandResponse(const CommandResponse &response)
{
    *this = response;
}

/// Move c'tor
CommandResponse::CommandResponse(CommandResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
CommandResponse& CommandResponse::operator=(const CommandResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<CommandResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
CommandResponse& CommandResponse::operator=(CommandResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Reset class
void CommandResponse::clear() noexcept
{
    pImpl = std::make_unique<CommandResponseImpl> ();
}

/// Destructor
CommandResponse::~CommandResponse() = default;

///  Convert message
std::string CommandResponse::toMessage() const
{
    return toCBORMessage(*this);
}

void CommandResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void CommandResponse::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    *this = fromCBORMessage(message, length);
}

/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> CommandResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandResponse> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    CommandResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<CommandResponse> ();
    return result;
}

/// Response
void CommandResponse::setResponse(const std::string &response)
{
    pImpl->mResponse = response;
    pImpl->mHaveResponse = true;
}

std::string CommandResponse::getResponse() const
{
    if (!haveResponse()){throw std::runtime_error("Response not set");}
    return pImpl->mResponse;
}

bool CommandResponse::haveResponse() const noexcept
{
    return pImpl->mHaveResponse;
}

/// Return code 
void CommandResponse::setReturnCode(const CommandReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
    pImpl->mHaveReturnCode = true;
}

CommandReturnCode CommandResponse::getReturnCode() const
{
    if (!haveReturnCode()){throw std::runtime_error("Return code not set");}
    return pImpl->mReturnCode;
}

bool CommandResponse::haveReturnCode() const noexcept
{
    return pImpl->mHaveReturnCode;
}


/// Message type
std::string CommandResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

