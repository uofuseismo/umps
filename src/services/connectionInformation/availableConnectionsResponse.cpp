#include <iostream>
#include <nlohmann/json.hpp>
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/authentication/enums.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

#define MESSAGE_TYPE "UMPS::Services::ConnectionInformation::AvailableConnectionsResponse"
#define MESSAGE_VERSION "1.0.0"

namespace
{

void throwOnInvalidDetails(const Details &detail)
{
    if (!detail.haveName())
    {
        throw std::invalid_argument("Name of connection not set");
    }
    if (!detail.haveConnectionType())
    {
        throw std::invalid_argument("Connection type not set");
    }
    if (detail.getSocketType() == SocketType::Unknown)
    {
        throw std::invalid_argument("Unknown socket information");
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

    auto socketType = detail.getSocketType();
    obj["SocketType"] = static_cast<int> (socketType);
    if (socketType == SocketType::Dealer)
    {
        auto socket = detail.getDealerSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::Publisher)
    {
        auto socket = detail.getPublisherSocketDetails(); 
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::Subscriber)
    {
        auto socket = detail.getSubscriberSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::Reply)
    {
        auto socket = detail.getReplySocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::Request)
    {
        auto socket = detail.getRequestSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::Router)
    {
        auto socket = detail.getRouterSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::XPublisher)
    {
        auto socket = detail.getXPublisherSocketDetails(); 
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }   
    else if (socketType == SocketType::XSubscriber)
    {
        auto socket = detail.getXSubscriberSocketDetails();
        obj["Address"] = socket.getAddress();
        obj["ConnectOrBind"] = static_cast<int> (socket.getConnectOrBind());
        obj["SecurityLevel"] = static_cast<int> (socket.getSecurityLevel());
        obj["MinimumPrivileges"]
            = static_cast<int> (socket.getMinimumUserPrivileges());
    }
    else if (socketType == SocketType::Proxy)
    {
        auto proxy = detail.getProxySocketDetails();
        auto frontendType = proxy.getFrontendSocketType();
        auto backendType  = proxy.getBackendSocketType();
        if (frontendType == SocketType::XSubscriber)
        {
            auto socket = proxy.getXSubscriberFrontend(); 
            obj["FrontendAddress"] = socket.getAddress();
            obj["FrontendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["FrontendConnectOrBind"]
                = static_cast<int> (socket.getConnectOrBind());
            obj["FrontendSecurityLevel"]
                = static_cast<int> (socket.getSecurityLevel());
            obj["FrontendMinimumPrivileges"]
                = static_cast<int> (socket.getMinimumUserPrivileges());
        }
        else if (frontendType == SocketType::Router)
        {
            auto socket = proxy.getRouterFrontend(); 
            obj["FrontendAddress"] = socket.getAddress();
            obj["FrontendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["FrontendConnectOrBind"]
                = static_cast<int> (socket.getConnectOrBind());
            obj["FrontendSecurityLevel"]
                = static_cast<int> (socket.getSecurityLevel());
            obj["FrontendMinimumPrivileges"]
                = static_cast<int> (socket.getMinimumUserPrivileges());
        }
        else
        {
            throw std::runtime_error("Unhandled frontend");
        }
 
        if (backendType == SocketType::XPublisher)
        {
            auto socket = proxy.getXPublisherBackend();
            obj["BackendAddress"] = socket.getAddress();
            obj["BackendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["BackendConnectOrBind"]
                = static_cast<int> (socket.getConnectOrBind());
            obj["BackendSecurityLevel"]
                = static_cast<int> (socket.getSecurityLevel());
            obj["BackendMinimumPrivileges"]
                = static_cast<int> (socket.getMinimumUserPrivileges());
        }
        else if (backendType == SocketType::Dealer)
        {
            auto socket = proxy.getDealerBackend();
            obj["BackendAddress"] = socket.getAddress();
            obj["BackendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["BackendConnectOrBind"]
                = static_cast<int> (socket.getConnectOrBind()); 
            obj["BackendSecurityLevel"]
                = static_cast<int> (socket.getSecurityLevel());
            obj["BackendMinimumPrivileges"]
                = static_cast<int> (socket.getMinimumUserPrivileges());
        }
        else if (backendType == SocketType::Router)
        {
            auto socket = proxy.getRouterBackend(); 
            obj["BackendAddress"] = socket.getAddress();
            obj["BackendSocketType"]
                = static_cast<int> (socket.getSocketType());
            obj["BackendConnectOrBind"]
                = static_cast<int> (socket.getConnectOrBind());
            obj["BackendSecurityLevel"]
                = static_cast<int> (socket.getSecurityLevel());
            obj["BackendMinimumPrivileges"]
                = static_cast<int> (socket.getMinimumUserPrivileges());
        }
        else
        {
            throw std::runtime_error("Unhandled backend");
        }
    }
    else
    {
        if (socketType != SocketType::Unknown)
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
        obj["ConnectionType"] = nullptr;
    }

    //auto privileges = static_cast<int> (detail.getUserPrivileges());
    //obj["UserPrivileges"] = privileges;

    auto securityLevel = static_cast<int> (detail.getSecurityLevel());
    obj["SecurityLevel"] = securityLevel;

    return obj;
}

Details objectToDetails(const nlohmann::json &obj)
{
    Details details;
    if (!obj["Name"].is_null()){details.setName(obj["Name"]);}

    auto socketType = static_cast<SocketType> (obj["SocketType"].get<int> ());
    if (socketType == SocketType::Publisher)
    {
        SocketDetails::Publisher socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind>
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::Subscriber)
    {
        SocketDetails::Subscriber socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ()); 
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket); 
    }
    else if (socketType == SocketType::Reply)
    {
        SocketDetails::Reply socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::Request)
    {
        SocketDetails::Request socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::Router)
    {
        SocketDetails::Router socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::Dealer)
    {
        SocketDetails::Dealer socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges> 
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::XPublisher)
    {
        SocketDetails::XPublisher socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::XSubscriber)
    {
        SocketDetails::XSubscriber socket;
        auto address = obj["Address"];
        auto corb = static_cast<ConnectOrBind> 
                    (obj["ConnectOrBind"].get<int> ());
        auto level = static_cast<UAuth::SecurityLevel>
                     (obj["SecurityLevel"].get<int> ());
        auto privileges = static_cast<UAuth::UserPrivileges>
                          (obj["MinimumPrivileges"].get<int> ());
        socket.setAddress(address);
        socket.setSecurityLevel(level);
        socket.setConnectOrBind(corb);
        socket.setMinimumUserPrivileges(privileges);
        details.setSocketDetails(socket);
    }
    else if (socketType == SocketType::Proxy)
    {
        SocketDetails::Proxy proxy;
        auto frontendSocketType = static_cast<SocketType>
            (obj["FrontendSocketType"].get<int> ());
        auto backendSocketType = static_cast<SocketType>
            (obj["BackendSocketType"].get<int> ());
        if (frontendSocketType == SocketType::XSubscriber &&
            backendSocketType  == SocketType::XPublisher)
        {
            SocketDetails::XSubscriber frontendSocket;
            SocketDetails::XPublisher  backendSocket;
            auto frontendAddress = obj["FrontendAddress"];
            auto frontendLevel = static_cast<UAuth::SecurityLevel>
                                 (obj["FrontendSecurityLevel"].get<int> ());
            auto frontendCorb = static_cast<ConnectOrBind> 
                                (obj["FrontendConnectOrBind"].get<int> ());
            auto frontendPrivileges = static_cast<UAuth::UserPrivileges>
                                 (obj["FrontendMinimumPrivileges"].get<int> ());
            frontendSocket.setAddress(frontendAddress);
            frontendSocket.setSecurityLevel(frontendLevel);
            frontendSocket.setConnectOrBind(frontendCorb);
            frontendSocket.setMinimumUserPrivileges(frontendPrivileges);
            auto backendAddress = obj["BackendAddress"];
            auto backendLevel = static_cast<UAuth::SecurityLevel>
                                (obj["BackendSecurityLevel"].get<int> ());
            auto backendCorb = static_cast<ConnectOrBind> 
                               (obj["BackendConnectOrBind"].get<int> ());
            auto backendPrivileges = static_cast<UAuth::UserPrivileges>
                                 (obj["BackendMinimumPrivileges"].get<int> ());
            backendSocket.setAddress(backendAddress); 
            backendSocket.setSecurityLevel(backendLevel);
            backendSocket.setConnectOrBind(backendCorb);
            backendSocket.setMinimumUserPrivileges(backendPrivileges);
            proxy.setSocketPair(std::pair(frontendSocket, backendSocket));
        }
        else if (frontendSocketType == SocketType::Router &&
                 backendSocketType  == SocketType::Dealer)
        {
            SocketDetails::Router frontendSocket;
            SocketDetails::Dealer  backendSocket;
            auto frontendAddress = obj["FrontendAddress"];
            auto frontendLevel = static_cast<UAuth::SecurityLevel>
                                 (obj["FrontendSecurityLevel"].get<int> ());
            auto frontendCorb = static_cast<ConnectOrBind> 
                                (obj["FrontendConnectOrBind"].get<int> ());
            auto frontendPrivileges = static_cast<UAuth::UserPrivileges>
                                 (obj["FrontendMinimumPrivileges"].get<int> ());
            frontendSocket.setAddress(frontendAddress);
            frontendSocket.setSecurityLevel(frontendLevel);
            frontendSocket.setConnectOrBind(frontendCorb);
            frontendSocket.setMinimumUserPrivileges(frontendPrivileges);
            auto backendAddress = obj["BackendAddress"];
            auto backendLevel = static_cast<UAuth::SecurityLevel>
                                (obj["BackendSecurityLevel"].get<int> ());
            auto backendCorb = static_cast<ConnectOrBind> 
                               (obj["BackendConnectOrBind"].get<int> ());
            auto backendPrivileges = static_cast<UAuth::UserPrivileges>
                                 (obj["BackendMinimumPrivileges"].get<int> ());
            backendSocket.setAddress(backendAddress); 
            backendSocket.setSecurityLevel(backendLevel);
            backendSocket.setConnectOrBind(backendCorb);
            backendSocket.setMinimumUserPrivileges(backendPrivileges);
            proxy.setSocketPair(std::pair(frontendSocket, backendSocket));
        }
        else if (frontendSocketType == SocketType::Router &&
                 backendSocketType  == SocketType::Router)
        {
            SocketDetails::Router frontendSocket;
            SocketDetails::Router  backendSocket;
            auto frontendAddress = obj["FrontendAddress"];
            auto frontendLevel = static_cast<UAuth::SecurityLevel>
                                 (obj["FrontendSecurityLevel"].get<int> ());
            auto frontendCorb = static_cast<ConnectOrBind> 
                                (obj["FrontendConnectOrBind"].get<int> ());
            auto frontendPrivileges = static_cast<UAuth::UserPrivileges>
                                 (obj["FrontendMinimumPrivileges"].get<int> ());
            frontendSocket.setAddress(frontendAddress);
            frontendSocket.setSecurityLevel(frontendLevel);
            frontendSocket.setConnectOrBind(frontendCorb);
            frontendSocket.setMinimumUserPrivileges(frontendPrivileges);
            auto backendAddress = obj["BackendAddress"];
            auto backendLevel = static_cast<UAuth::SecurityLevel>
                                (obj["BackendSecurityLevel"].get<int> ());
            auto backendCorb = static_cast<ConnectOrBind> 
                               (obj["BackendConnectOrBind"].get<int> ());
            auto backendPrivileges = static_cast<UAuth::UserPrivileges>
                                 (obj["BackendMinimumPrivileges"].get<int> ());
            backendSocket.setAddress(backendAddress); 
            backendSocket.setSecurityLevel(backendLevel);
            backendSocket.setConnectOrBind(backendCorb);
            backendSocket.setMinimumUserPrivileges(backendPrivileges);
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
        if (socketType != SocketType::Unknown)
        {
            throw std::runtime_error("Unhandled socket");
        }
    }

    if (!obj["ConnectionType"].is_null())
    {
        auto connectionType = static_cast<ConnectionType>
                              (obj["ConnectionType"].get<int> ());
        details.setConnectionType(connectionType);
    }
    //if (!obj["UserPrivileges"].is_null())
    //{
    //    auto privileges = static_cast<UAuth::UserPrivileges>
    //                      (obj["UserPrivileges"].get<int> ());
    //    details.setUserPrivileges(privileges);
    //}
    if (!obj["SecurityLevel"].is_null())
    {
        auto security = static_cast<UAuth::SecurityLevel>
                        (obj["SecurityLevel"].get<int> ());
        details.setSecurityLevel(security);
    }
    return details;
}

nlohmann::json toJSONObject(const AvailableConnectionsResponse &response)
{
    nlohmann::json obj;
    auto details = response.getDetails();
    obj["MessageType"] = response.getMessageType();
    obj["MessageVersion"] = response.getMessageVersion();
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

AvailableConnectionsResponse
    objectToResponse(const nlohmann::json &obj)
{
    AvailableConnectionsResponse response;
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

AvailableConnectionsResponse
    fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToResponse(obj);
}

AvailableConnectionsResponse fromCBORMessage(const uint8_t *message,
                                             const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToResponse(obj);
}

}

///--------------------------------------------------------------------------///
///                                 Implementation                           ///
///--------------------------------------------------------------------------///

class AvailableConnectionsResponse::ResponseImpl
{
public:
    std::vector<Details> mDetails;
    ReturnCode mReturnCode = ReturnCode::SUCCESS; 
};

/// C'tor
AvailableConnectionsResponse::AvailableConnectionsResponse() :
    pImpl(std::make_unique<ResponseImpl> ())
{
}

/// Copy c'tor
AvailableConnectionsResponse::AvailableConnectionsResponse(
    const AvailableConnectionsResponse &response)
{
    *this = response;
}

/// Move c'tor 
AvailableConnectionsResponse::AvailableConnectionsResponse(
    AvailableConnectionsResponse &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment 
AvailableConnectionsResponse& AvailableConnectionsResponse::operator=(
    const AvailableConnectionsResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<ResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
AvailableConnectionsResponse& AvailableConnectionsResponse::operator=(
    AvailableConnectionsResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
AvailableConnectionsResponse::~AvailableConnectionsResponse() = default;

/// Reset class
void AvailableConnectionsResponse::clear() noexcept
{
    pImpl = std::make_unique<ResponseImpl> (); 
}

/// Message type
std::string AvailableConnectionsResponse::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Message version
std::string AvailableConnectionsResponse::getMessageVersion() const noexcept
{
    return MESSAGE_VERSION;
}

/// Clone
std::unique_ptr<UMPS::MessageFormats::IMessage> 
    AvailableConnectionsResponse::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableConnectionsResponse> (*this);
    return result;
}

/// Create instance
std::unique_ptr<UMPS::MessageFormats::IMessage>
    AvailableConnectionsResponse::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<AvailableConnectionsResponse> (); 
    return result;
}

/// Convert message
std::string AvailableConnectionsResponse::toMessage() const
{
    return toCBOR();
}

void AvailableConnectionsResponse::setDetails(
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

std::vector<Details> AvailableConnectionsResponse::getDetails() const noexcept
{
    return pImpl->mDetails;
}

/// Return code
void AvailableConnectionsResponse::setReturnCode(
    const ReturnCode returnCode) noexcept
{
    pImpl->mReturnCode = returnCode;
}

ReturnCode AvailableConnectionsResponse::getReturnCode() const noexcept
{
    return pImpl->mReturnCode;
}

void AvailableConnectionsResponse::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());
}

void AvailableConnectionsResponse::fromMessage(const char *messageIn,
                                               const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// From CBOR
void AvailableConnectionsResponse::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void AvailableConnectionsResponse::fromCBOR(const uint8_t *data,
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
void AvailableConnectionsResponse::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// Create JSON
std::string AvailableConnectionsResponse::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string AvailableConnectionsResponse::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}
