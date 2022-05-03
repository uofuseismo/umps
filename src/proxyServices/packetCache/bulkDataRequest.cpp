#include <iostream>
#include <cmath>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/packetCache/bulkDataRequest.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "private/applications/packetCache.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::PacketCache::BulkDataRequest"

using namespace UMPS::ProxyServices::PacketCache;

namespace
{

bool operator==(const DataRequest &lhs, const DataRequest &rhs)
{
    if (lhs.getIdentifier() != rhs.getIdentifier()){return false;}
    auto [lhsStartTime, lhsEndTime] = lhs.getQueryTimes();
    auto [rhsStartTime, rhsEndTime] = lhs.getQueryTimes();
    if (std::abs(lhsStartTime - rhsStartTime) > 1.e-6){return false;}
    if (std::abs(lhsEndTime - rhsEndTime) > 1.e-6){return false;}
    if (lhs.haveNetwork() == rhs.haveNetwork())
    {
        if (lhs.haveNetwork())
        {
            if (lhs.getNetwork() != rhs.getNetwork()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveStation() == rhs.haveStation())
    {
        if (lhs.haveStation())
        {
            if (lhs.getStation() != rhs.getStation()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveChannel() == rhs.haveChannel())
    {
        if (lhs.haveChannel())
        {
            if (lhs.getChannel() != rhs.getChannel()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveLocationCode() == rhs.haveLocationCode())
    {
        if (lhs.haveLocationCode())
        {
            if (lhs.getLocationCode() != rhs.getLocationCode()){return false;}
        }
    }
    else
    {
        return false;
    }

    return true;
}

nlohmann::json toJSONObject(const BulkDataRequest &request)
{
    nlohmann::json obj;
    obj["MessageType"] = request.getMessageType();
    auto nRequests = request.getNumberOfDataRequests();
    const auto requestsPtr = request.getDataRequestsPointer();
    if (nRequests > 0)
    {
        nlohmann::json requestObjects;
        for (int i = 0; i < nRequests; ++i)
        {
            nlohmann::json requestObject;
            // Essential stuff (this will throw): 
            requestObject["Network"] = requestsPtr[i].getNetwork();
            requestObject["Station"] = requestsPtr[i].getStation();
            requestObject["Channel"] = requestsPtr[i].getChannel();
            requestObject["LocationCode"] = requestsPtr[i].getLocationCode();
            // Other stuff (times and identifier)
            auto [startTime, endTime] = requestsPtr[i].getQueryTimes();
            requestObject["StartTime"] = startTime;
            requestObject["EndTime"] = endTime;
            requestObject["Identifier"] = requestsPtr[i].getIdentifier();
            // Append
            requestObjects.push_back(std::move(requestObject));
         }
         obj["Requests"] = requestObjects;
    }
    else
    {
         obj["Requests"] = nullptr;
    }
    obj["NumberOfRequests"] = nRequests;
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

BulkDataRequest objectToBulkDataRequest(const nlohmann::json &obj)
{
    BulkDataRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {   
        throw std::invalid_argument("Message has invalid message type");
    }
    // Loop through requests
    auto nRequests = obj["NumberOfRequests"].get<int> ();
    request.setIdentifier(obj["Identifier"]);
    if (nRequests > 0)
    {
        auto requestObjects = obj["Requests"];
        for (const auto &requestObject : requestObjects)
        {
            DataRequest dataRequest;
            // Essential stuff
            dataRequest.setNetwork(requestObject["Network"]
                       .get<std::string> ());
            dataRequest.setStation(requestObject["Station"]
                       .get<std::string> ());
            dataRequest.setChannel(requestObject["Channel"]
                       .get<std::string> ());
            dataRequest.setLocationCode(requestObject["LocationCode"]
                       .get<std::string> ());
            // Other stuff
            auto startTime = requestObject["StartTime"].get<double> ();
            auto endTime = requestObject["EndTime"].get<double> ();
            dataRequest.setQueryTimes(std::pair(startTime, endTime));
            dataRequest.setIdentifier(requestObject["Identifier"]
                                     .get<uint64_t> ());
            // Append it
            request.addDataRequest(dataRequest);
         }
    }
    return request;
}

BulkDataRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToBulkDataRequest(obj);
}

BulkDataRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToBulkDataRequest(obj);
}

}

class BulkDataRequest::BulkDataRequestImpl
{
public:
    std::vector<DataRequest> mRequests;
    uint64_t mIdentifier = 0;
};

/// C'tor
BulkDataRequest::BulkDataRequest() :
    pImpl(std::make_unique<BulkDataRequestImpl> ())
{
}

/// Copy assignment
BulkDataRequest::BulkDataRequest(const BulkDataRequest &request)
{
    *this = request;
}

/// Move assignment
BulkDataRequest::BulkDataRequest(BulkDataRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
BulkDataRequest& BulkDataRequest::operator=(const BulkDataRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<BulkDataRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
BulkDataRequest& BulkDataRequest::operator=(BulkDataRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Reset class
void BulkDataRequest::clear() noexcept
{
    pImpl = std::make_unique<BulkDataRequestImpl> ();
}

/// Destructor
BulkDataRequest::~BulkDataRequest() = default;

/// Message type
std::string BulkDataRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Data requests
void BulkDataRequest::addDataRequest(const DataRequest &request)
{
    // Validate basic information
    if (!request.haveNetwork())
    {
        throw std::invalid_argument("Network not set");
    }
    if (!request.haveStation())
    {
        throw std::invalid_argument("Station not set");
    }
    if (!request.haveChannel())
    {
        throw std::invalid_argument("Channel not set");
    }
    if (!request.haveLocationCode())
    {
        throw std::invalid_argument("Location code not set");
    }
    // Check for existance
    if (haveDataRequest(request))
    {
        throw std::invalid_argument("Request exists");
    }
    // It's new - add it
    pImpl->mRequests.push_back(request);
}

void BulkDataRequest::clearDataRequests() noexcept
{
    pImpl->mRequests.clear();
}

bool BulkDataRequest::haveDataRequest(const DataRequest &request) const noexcept
{
    for (const auto &r : pImpl->mRequests)
    {
        if (r == request){return true;}
    }
    return false;
}


std::vector<DataRequest> BulkDataRequest::getDataRequests() const noexcept
{
    return pImpl->mRequests;
}

const DataRequest *BulkDataRequest::getDataRequestsPointer() const noexcept
{
    return pImpl->mRequests.data();
}

int BulkDataRequest::getNumberOfDataRequests() const noexcept
{
    return static_cast<int> (pImpl->mRequests.size());
}

/// Identifier
void BulkDataRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t BulkDataRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string BulkDataRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string BulkDataRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void BulkDataRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void BulkDataRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void BulkDataRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string BulkDataRequest::toMessage() const
{
    return toCBOR();
}

void BulkDataRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> BulkDataRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<BulkDataRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    BulkDataRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<BulkDataRequest> (); 
    return result;
}

