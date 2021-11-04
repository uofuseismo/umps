#include <iostream>
#include <nlohmann/json.hpp>
#include "umps/services/connectionInformation/availableBroadcastsResponse.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/messaging/authentication/enums.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Messaging::Authentication;

#define MESSAGE_TYPE "UMPS::Services::ConnectionInformation::AvailableBroadcastsResponse"

namespace
{

void throwOnInvalidDetails(const Details &detail)
{
    if (!detail.haveName())
    {
        throw std::invalid_argument("Name of broadcast not set");
    }
    if (!detail.haveConnectionString())
    {
        throw std::invalid_argument("Connection string not set");
    }
}

nlohmann::json detailsToJSONObject(const Details &detail)
{
    nlohmann::json obj;
    if (detail.haveName())
    {
        obj["Name"] = detail.getName();
    }
    else
    {
        obj["Name"] = nullptr;
    }

    if (detail.haveConnectionString())
    {
        obj["ConnectionString"] = detail.getConnectionString();
    }
    else
    {
        obj["ConnectionString"] = nullptr;
    }
    auto socketType = detail.getSocketType();
    obj["SocketType"] = static_cast<int> (socketType);
    if (socketType == SocketType::PUBLISHER)
    {
        auto socket = detail.getPublisherSocketDetails(); 
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.connectOrBind());  
    }
    else if (socketType == SocketType::SUBSCRIBER)
    {
        auto socket = detail.getSubscriberSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.connectOrBind());  
    }
    else if (socketType == SocketType::REQUEST)
    {
        auto socket = detail.getRequestSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.connectOrBind());
    }
    else if (socketType == SocketType::ROUTER)
    {
        auto socket = detail.getRouterSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.connectOrBind());
    }
    else if (socketType == SocketType::XPUBLISHER)
    {
        auto socket = detail.getXPublisherSocketDetails(); 
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.connectOrBind());  
    }   
    else if (socketType == SocketType::XSUBSCRIBER)
    {
        auto socket = detail.getXSubscriberSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.connectOrBind());
    }
    else if (socketType == SocketType::PROXY)
    {
        auto proxy = detail.getProxySocketDetails();
        auto frontendType = proxy.getFrontendSocketType();
        auto backendType  = proxy.getBackendSocketType();
        if (frontendType == SocketType::XSUBSCRIBER)
        {
            auto socket = proxy.getXSubscriberFrontend(); 
            obj["FrontendAddress"] = socket.getAddress();
            obj["FrontendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["FrontendConnectOrBind"]
                = static_cast<int> (socket.connectOrBind());
        }
        else
        {
            throw std::runtime_error("Unhandled frontend");
        }
 
        if (backendType == SocketType::XPUBLISHER)
        {
            auto socket = proxy.getXPublisherBackend();
            obj["BackendAddress"] = socket.getAddress();
            obj["BackendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["BackendConnectOrBind"]
                = static_cast<int> (socket.connectOrBind());
        }
        else
        {
            throw std::runtime_error("Unhandled backend");
        }
    }
    else
    {
        if (socketType != SocketType::UNKNOWN)
        {
            throw std::runtime_error("Unhandled socket");
        }
    }

    if (detail.haveConnectionType())
    {
        obj["ConnectionType"] = detail.getConnectionType();
    }
    else
    {
        obj["ConnectionString"] = nullptr;
    }

    auto privileges = static_cast<int> (detail.getUserPrivileges());
    obj["UserPrivileges"] = privileges;

    auto securityLevel = static_cast<int> (detail.getSecurityLevel());
    obj["SecurityLevel"] = securityLevel;

    return obj;
}

Details objectToDetails(const nlohmann::json &obj)
{
    Details details;
    if (!obj["Name"].is_null()){details.setName(obj["Name"]);}
    if (!obj["ConnectionString"].is_null())
    {
        details.setConnectionString(obj["ConnectionString"]);
    }
    if (!obj["ConnectionType"].is_null())
    {
        auto connectionType = static_cast<ConnectionType>
                              (obj["ConnectionType"].get<int> ());
        details.setConnectionType(connectionType);
    }
    auto socketType = static_cast<SocketType> (obj["SocketType"].get<int> ());
    if (socketType == SocketType::PUBLISHER)
    {
        SocketDetails::Publisher socket;
        auto address = obj["Address"];
        socket.setAddress(address);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::SUBSCRIBER)
    {
        SocketDetails::Subscriber socket;
        auto address = obj["Address"];
        socket.setAddress(address);
        details.setSocketDetails(socket); 
    }
    else if (socketType == SocketType::REQUEST)
    {
        SocketDetails::Request socket;
        auto address = obj["Address"];
        socket.setAddress(address);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::ROUTER)
    {
        SocketDetails::Router socket;
        auto address = obj["Address"];
        socket.setAddress(address);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::XPUBLISHER)
    {
        SocketDetails::XPublisher socket;
        auto address = obj["Address"];
        socket.setAddress(address);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::XSUBSCRIBER)
    {
        SocketDetails::XSubscriber socket;
        auto address = obj["Address"];
        socket.setAddress(address);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::PROXY)
    {
        SocketDetails::Proxy proxy;
        auto frontendSocketType = static_cast<SocketType>
            (obj["FrontendSocketType"].get<int> ());
        auto backendSocketType = static_cast<SocketType>
            (obj["BackendSocketType"].get<int> ());
        if (frontendSocketType == SocketType::XSUBSCRIBER &&
            backendSocketType  == SocketType::XPUBLISHER)
        {
            SocketDetails::XSubscriber frontendSocket;
            SocketDetails::XPublisher  backendSocket;
            auto frontendAddress = obj["FrontendAddress"];
            frontendSocket.setAddress(frontendAddress);
            auto backendAddress = obj["BackendAddress"];
            backendSocket.setAddress(backendAddress); 
            proxy.setSocketPair(std::pair(frontendSocket, backendSocket));
        }
        else
        {
            throw std::runtime_error("Unhandled frontend/backend pair");
        }
        details.setSocketDetails(proxy);
    }
    else
    {
        if (socketType != SocketType::UNKNOWN)
        {
            throw std::runtime_error("Unhandled socket");
        }
    }

    if (!obj["UserPrivileges"].is_null())
    {
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["UserPrivileges"].get<int> ());
        details.setUserPrivileges(privileges);
    }
    if (!obj["SecurityLevel"].is_null())
    {
        auto security = static_cast<UAuth::SecurityLevel>
                        (obj["SecurityLevel"].get<int> ());
        details.setSecurityLevel(security);
    }
    return details;
}

nlohmann::json toJSONObject(const AvailableBroadcastsResponse &response)
{
    nlohmann::json obj;
    auto details = response.getDetails();
    obj["MessageType"] = response.getMessageType();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    nlohmann::json detailsObj;
    for (const auto &detail : details)
    {
        auto detailObj = detailsToJSONObject(detail);
        detailsObj.push_back(detailObj);
    }
    obj["Details"] = detailsObj;
    return obj;
}

AvailableBroadcastsResponse
    objectToResponse(const nlohmann::json &obj)
{
    AvailableBroadcastsResponse response;
    if (obj["MessageType"] != response.getMessageType())
    {   
        throw std::invalid_argument("Message has invalid message type");
    }
    auto returnCode = static_cast<ReturnCode> (obj["ReturnCode"].get<int> ());
    response.setReturnCode(returnCode);
    auto detailsObj = obj["Details"];
    std::vector<Details> details;
    for (const auto &detailObj : detailsObj)
    {
        details.push_back(objectToDetails(detailObj));
    }
    response.setDetails(details);
    return response;
}

AvailableBroadcastsResponse
    fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

AvailableBroadcastsResponse fromCBORMessage(const uint8_t *message,
                                            const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

///--------------------------------------------------------------------------///
///                                 Implementation                           ///
///--------------------------------------------------------------------------///

class AvailableBroadcastsResponse::ResponseImpl
{
public:
    std::vector<Details> mDetails;
    ReturnCode mReturnCode = ReturnCode::SUCCESS; 
};

/// C'tor
AvailableBroadcastsResponse::AvailableBroadcastsResponse() :
    pImpl(std::make_unique<ResponseImpl> ())
{
}

/// Copy c'tor
AvailableBroadcastsResponse::AvailableBroadcastsResponse(
    const AvailableBroadcastsResponse &response)
{
    *this = response;
}

/// Move c'tor 
AvailableBroadcastsResponse::AvailableBroadcastsResponse(
    AvailableBroadcastsResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment 
AvailableBroadcastsResponse& AvailableBroadcastsResponse::operator=(
    const AvailableBroadcastsResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<ResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
AvailableBroadcastsResponse& AvailableBroadcastsResponse::operator=(
    AvailableBroadcastsResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
AvailableBroadcastsResponse::~AvailableBroadcastsResponse() = default;

/// Reset class
void AvailableBroadcastsResponse::clear() noexcept
{
    pImpl = std::make_unique<ResponseImpl> (); 
}

/// Message type
std::string AvailableBroadcastsResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Clone
std::unique_ptr<UMPS::MessageFormats::IMessage> 
    AvailableBroadcastsResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableBroadcastsResponse> (*this);
    return result;
}

/// Create instance
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableBroadcastsResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableBroadcastsResponse> (); 
    return result;
}

/// Convert message
std::string AvailableBroadcastsResponse::toMessage() const
{
    return toCBOR();
}

/// Details
void AvailableBroadcastsResponse::setDetail(const Details &detail)
{
    throwOnInvalidDetails(detail);
    pImpl->mDetails.clear();
    pImpl->mDetails.push_back(detail);
}

void AvailableBroadcastsResponse::setDetails(
    const std::vector<Details> &details)
{
    // No details to copy
    if (details.empty()){return;}
    // Check the details
    std::vector<Details> detailsCopy;
    for (const auto &detail : details)
    {
        throwOnInvalidDetails(detail);
    }
    pImpl->mDetails = details;
}

std::vector<Details> AvailableBroadcastsResponse::getDetails() const noexcept
{
    return pImpl->mDetails;
}

/// Return code
void AvailableBroadcastsResponse::setReturnCode(
    const ReturnCode returnCode) noexcept
{
    pImpl->mReturnCode = returnCode;
}

ReturnCode AvailableBroadcastsResponse::getReturnCode() const noexcept
{
    return pImpl->mReturnCode;
}

void AvailableBroadcastsResponse::fromMessage(const char *messageIn,
                                              const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// From CBOR
void AvailableBroadcastsResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void AvailableBroadcastsResponse::fromCBOR(const uint8_t *data,
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
void AvailableBroadcastsResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// Create JSON
std::string AvailableBroadcastsResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string AvailableBroadcastsResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}
