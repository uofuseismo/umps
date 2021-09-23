#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/services/packetCache/dataRequest.hpp"
#include "private/applications/packetCache.hpp"

#define MESSAGE_TYPE "UMPS::Services::PacketCache::DataRequest"

using namespace UMPS::Services::PacketCache;

namespace
{

nlohmann::json toJSONObject(const DataRequest &request)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    // Network/Station/Channel/Location
    obj["MessageType"] = request.getMessageType();
    obj["Network"] = request.getNetwork();
    obj["Station"] = request.getStation();
    obj["Channel"] = request.getChannel();
    obj["LocationCode"] = request.getLocationCode();
    // Other stuff (times and identifier)
    auto [startTime, endTime] = request.getQueryTimes();
    obj["StartTime"] = startTime;
    obj["EndTime"] = endTime;
    obj["Identifier"] = request.getIdentifier();
    return obj;
}

DataRequest objectToDataRequest(const nlohmann::json obj)
{
    DataRequest request;
    if (obj["MessageType"] != request.getMessageType())
    {   
        throw std::invalid_argument("Message has invalid message type");
    }   
    // Essential stuff
    request.setNetwork(obj["Network"].get<std::string> ());
    request.setStation(obj["Station"].get<std::string> ());
    request.setChannel(obj["Channel"].get<std::string> ());
    request.setLocationCode(obj["LocationCode"].get<std::string> ());
    // Other stuff
    auto startTime = obj["StartTime"].get<double> ();
    auto endTime = obj["EndTime"].get<double> ();
    request.setQueryTimes(std::pair(startTime, endTime));
    request.setIdentifier(obj["Identifier"]);
    return request;
}

DataRequest fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToDataRequest(obj);
}

DataRequest fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToDataRequest(obj);
}

}

class DataRequest::DataRequestImpl
{
public:
    std::string mNetwork;
    std::string mStation;
    std::string mChannel;
    std::string mLocationCode;
    uint64_t mIdentifier = 0;
    double mStartTime = std::numeric_limits<double>::lowest();
    double mEndTime = std::numeric_limits<double>::max();
};

/// C'tor
DataRequest::DataRequest() :
    pImpl(std::make_unique<DataRequestImpl> ())
{
}

/// Copy assignment
DataRequest::DataRequest(const DataRequest &request)
{
    *this = request;
}

/// Move assignment
DataRequest::DataRequest(DataRequest &&request) noexcept
{
    *this = std::move(request);
}

/// Copy assignment
DataRequest& DataRequest::operator=(const DataRequest &request)
{
    if (&request == this){return *this;}
    pImpl = std::make_unique<DataRequestImpl> (*request.pImpl);
    return *this;
}

/// Move assignment
DataRequest& DataRequest::operator=(DataRequest &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Reset class
void DataRequest::clear() noexcept
{
    pImpl = std::make_unique<DataRequestImpl> ();
}

/// Destructor
DataRequest::~DataRequest() = default;

/// Message type
std::string DataRequest::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Network
void DataRequest::setNetwork(const std::string &network)
{
    if (isEmpty(network)){throw std::invalid_argument("Network is empty");}
    pImpl->mNetwork = network;
}

std::string DataRequest::getNetwork() const
{
    if (!haveNetwork()){throw std::runtime_error("Network not set yet");}
    return pImpl->mNetwork;
}

bool DataRequest::haveNetwork() const noexcept
{
    return !pImpl->mNetwork.empty();
}

/// Station
void DataRequest::setStation(const std::string &station)
{
    if (isEmpty(station)){throw std::invalid_argument("Station is empty");}
    pImpl->mStation = station;
}

std::string DataRequest::getStation() const
{
    if (!haveStation()){throw std::runtime_error("Station not set yet");}
    return pImpl->mStation;
}

bool DataRequest::haveStation() const noexcept
{
    return !pImpl->mStation.empty();
}

/// Channel
void DataRequest::setChannel(const std::string &channel)
{
    if (isEmpty(channel)){throw std::invalid_argument("Channel is empty");}
    pImpl->mChannel = channel;
}

std::string DataRequest::getChannel() const
{
    if (!haveChannel()){throw std::runtime_error("Channel not set yet");}
    return pImpl->mChannel;
}

bool DataRequest::haveChannel() const noexcept
{
    return !pImpl->mChannel.empty();
}

/// Location code
void DataRequest::setLocationCode(const std::string &location)
{
    if (isEmpty(location)){throw std::invalid_argument("location is empty");}
    pImpl->mLocationCode = location;
}

std::string DataRequest::getLocationCode() const
{
    if (!haveLocationCode())
    {
        throw std::runtime_error("Location code not set yet");
    }
    return pImpl->mLocationCode;
}

bool DataRequest::haveLocationCode() const noexcept
{
    return !pImpl->mLocationCode.empty();
}

/// Start/end times
void DataRequest::setQueryTimes(const std::pair<double, double> &times)
{
    if (times.first >= times.second)
    {
        throw std::invalid_argument("times.first = "
                                   + std::to_string(times.first)
                                   + " must be less than time.second = "
                                   + std::to_string(times.second));
    }
    pImpl->mStartTime = times.first;
    pImpl->mEndTime = times.second;
}

void DataRequest::setQueryTime(const double time)
{
    if (time == std::numeric_limits<double>::max())
    {
        throw std::invalid_argument(
            "Query start time can't be set to max double");
    }
    setQueryTimes(
        std::pair<double, double> (time, std::numeric_limits<double>::max()));
}

std::pair<double, double> DataRequest::getQueryTimes() const noexcept
{
    return std::pair(pImpl->mStartTime, pImpl->mEndTime);
}

/// Identifier
void DataRequest::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

uint64_t DataRequest::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Create JSON
std::string DataRequest::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string DataRequest::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
void DataRequest::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void DataRequest::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void DataRequest::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string DataRequest::toMessage() const
{
    return toCBOR();
}

void DataRequest::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> DataRequest::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<DataRequest> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    DataRequest::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<DataRequest> (); 
    return result;
}

