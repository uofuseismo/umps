#include <nlohmann/json.hpp>
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"

using namespace UMPS::Services::ConnectionInformation;

#define MESSAGE_TYPE "UMPS::Services::ConnectionInformation::AvailableConnectionsRequest"
#define MESSAGE_VERSION "1.0.0"

namespace
{

nlohmann::json toJSONObject(const AvailableConnectionsRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    obj["MessageVersion"] = request.getMessageVersion();
    return obj;
}

AvailableConnectionsRequest objectToRequest(const nlohmann::json &obj)
{
    AvailableConnectionsRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {   
        throw std::invalid_argument("Message has invalid message type");
    }   
    return request;
}

AvailableConnectionsRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToRequest(obj);
}

AvailableConnectionsRequest fromCBORMessage(const uint8_t *message,
                                            const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToRequest(obj);
}

}

///--------------------------------------------------------------------------///
///                                 Implementation                           ///
///--------------------------------------------------------------------------///

class AvailableConnectionsRequest::RequestImpl
{
public:

};

/// C'tor
AvailableConnectionsRequest::AvailableConnectionsRequest() :
    pImpl(std::make_unique<RequestImpl> ())
{
}

/// Copy c'tor
AvailableConnectionsRequest::AvailableConnectionsRequest(
    const AvailableConnectionsRequest &request)
{
    *this = request;
}

/// Move c'tor 
AvailableConnectionsRequest::AvailableConnectionsRequest(
    AvailableConnectionsRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment 
AvailableConnectionsRequest& AvailableConnectionsRequest::operator=(
    const AvailableConnectionsRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<RequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
AvailableConnectionsRequest& AvailableConnectionsRequest::operator=(
    AvailableConnectionsRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
AvailableConnectionsRequest::~AvailableConnectionsRequest() = default;

/// Message type
std::string AvailableConnectionsRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string AvailableConnectionsRequest::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

/// Clone
std::unique_ptr<UMPS::MessageFormats::IMessage> 
    AvailableConnectionsRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableConnectionsRequest> (*this);
    return result;
}

/// Create instance
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableConnectionsRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableConnectionsRequest> (); 
    return result;
}

/// Convert message
std::string AvailableConnectionsRequest::toMessage() const
{
    return toCBOR();
}


void AvailableConnectionsRequest::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void AvailableConnectionsRequest::fromMessage(const char *messageIn,
                                             const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// From CBOR
void AvailableConnectionsRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void AvailableConnectionsRequest::fromCBOR(const uint8_t *data,
                                          const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// From JSON
void AvailableConnectionsRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// Create JSON
std::string AvailableConnectionsRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string AvailableConnectionsRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}
