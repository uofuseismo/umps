#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "private/applications/packetCache.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::PacketCache::DataResponse"

using namespace UMPS::ProxyServices::PacketCache;

namespace
{

template<class T>
nlohmann::json toJSONObject(
    const DataResponse<T> &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    auto nPackets = response.getNumberOfPackets();
    obj["NumberOfPackets"] = nPackets;
    auto packetsPointer = response.getPacketsPointer();
    if (nPackets > 0)
    {
        obj["Network"] = packetsPointer[0].getNetwork();
        obj["Station"] = packetsPointer[0].getStation();
        obj["Channel"] = packetsPointer[0].getChannel();
        obj["LocationCode"] = packetsPointer[0].getLocationCode();
        // Now the packets (these were sorted on time)
        nlohmann::json packetObjects;
        for (int ip = 0; ip < nPackets; ++ip)
        {
            nlohmann::json packetObject;
            packetObject["StartTime"]
                = packetsPointer[ip].getStartTime().count();
            packetObject["SamplingRate"] = packetsPointer[ip].getSamplingRate();
            auto nSamples = packetsPointer[ip].getNumberOfSamples();
            if (nSamples > 0)
            {
                packetObject["Data"] = packetsPointer[ip].getData();
            }
            else
            {
                packetObject["Data"] = nullptr;
            } 
            packetObjects.push_back(packetObject);
        }
        obj["Packets"] = packetObjects;
    }
    else
    {
        obj["Packets"] = nullptr;
    }
    obj["Identifier"] = response.getIdentifier();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

template<typename T>
DataResponse<T> objectToDataResponse(const nlohmann::json &obj)
{
    DataResponse<T> response;
    if (obj["MessageType"] != response.getMessageType())
    {   
        throw std::invalid_argument("Message has invalid message type");
    }   
    auto nPackets = obj["NumberOfPackets"].get<int> ();
    if (nPackets > 0)
    {
        auto network = obj["Network"].get<std::string> ();
        auto station = obj["Station"].get<std::string> ();
        auto channel = obj["Channel"].get<std::string> ();
        auto locationCode = obj["LocationCode"].get<std::string> ();
        std::vector<UMPS::MessageFormats::DataPacket<T>> packets;
        auto packetObjects = obj["Packets"];
        packets.reserve(nPackets);
        for (const auto &packetObject : packetObjects)
        {
            UMPS::MessageFormats::DataPacket<T> packet;
            packet.setNetwork(network);
            packet.setStation(station);
            packet.setChannel(channel);
            packet.setLocationCode(locationCode);
            packet.setSamplingRate(packetObject["SamplingRate"].get<double> ());
            auto startTime = packetObject["StartTime"].get<int64_t> (); 
            std::chrono::microseconds startTimeMuS{startTime};
            packet.setStartTime(startTimeMuS);
            if (!packetObject["Data"].is_null())
            {
                std::vector<T> data = packetObject["Data"];
                packet.setData(std::move(data));
            }
            packets.push_back(std::move(packet));
        }
        response.setPackets(std::move(packets));
    }
    response.setIdentifier(obj["Identifier"]);
    response.setReturnCode(static_cast<ReturnCode>
                           (obj["ReturnCode"].get<int> ()));
    return response;
}

template<typename T>
DataResponse<T> fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToDataResponse<T>(obj);
}

template<typename T>
DataResponse<T> fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToDataResponse<T>(obj);
}

template<typename T>
void checkPackets(const std::vector<UMPS::MessageFormats::DataPacket<T>> &packets)
{
    std::string name;
    for (const auto &packet : packets)
    {
        if (!packet.haveNetwork())
        {
            throw std::invalid_argument("Network not set on packet");
        }
        if (!packet.haveStation())
        {
            throw std::invalid_argument("Station not set on packet");
        }
        if (!packet.haveChannel())
        {
            throw std::invalid_argument("Channel not set on packet");
        }
        if (!packet.haveLocationCode())
        {
            throw std::invalid_argument("Location code not set on packet");
        }
        if (!packet.haveSamplingRate())
        {
            throw std::invalid_argument("Sampling rate not set");
        }
        // Check that names are consistent
        auto tempName = packet.getNetwork() + "." + packet.getStation() + "." 
                      + packet.getChannel() + "." + packet.getLocationCode();
        if (!name.empty()) 
        {
            if (tempName != name)
            {
                throw std::runtime_error("Inconsistent SNCL");
            }
        }
        else
        {
            name = tempName;
        }
    }
}

}

template<class T>
class DataResponse<T>::DataResponseImpl
{
public:
    void sortPackets()
    {
        if (!std::is_sorted(mPackets.begin(),
                            mPackets.end(),
                            [](const UMPS::MessageFormats::DataPacket<T> &a,
                               const UMPS::MessageFormats::DataPacket<T> &b)
                            {
                                return a.getStartTime() < b.getStartTime();
                            }))
        {
            std::sort(mPackets.begin(), mPackets.end(),
                      [](const UMPS::MessageFormats::DataPacket<T> &a,
                         const UMPS::MessageFormats::DataPacket<T> &b)
                       {
                           return a.getStartTime() < b.getStartTime();
                       });
        }
    }
    std::vector<UMPS::MessageFormats::DataPacket<T>> mPackets;
    uint64_t mIdentifier = 0;
    ReturnCode mReturnCode = ReturnCode::SUCCESS;
};

/// C'tor
template<class T>
DataResponse<T>::DataResponse() :
    pImpl(std::make_unique<DataResponseImpl> ())
{
}

/// Copy c'tor
template<class T>
DataResponse<T>::DataResponse(const DataResponse<T> &response)
{
    *this = response;
}

/// Move c'tor
template<class T>
DataResponse<T>::DataResponse(DataResponse<T> &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
template<class T>
DataResponse<T>& DataResponse<T>::operator=(const DataResponse<T> &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<DataResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
template<class T>
DataResponse<T>& DataResponse<T>::operator=(DataResponse<T> &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
template<class T>
DataResponse<T>::~DataResponse() = default;

/// Reset class
template<class T>
void DataResponse<T>::clear() noexcept
{
    pImpl = std::make_unique<DataResponseImpl> ();
}

/// Packets
template<class T>
void DataResponse<T>::setPackets(
    const std::vector<UMPS::MessageFormats::DataPacket<T>> &packets)
{
    checkPackets(packets);
    pImpl->mPackets = packets;
    pImpl->sortPackets();
}

template<class T>
void DataResponse<T>::setPackets(
    std::vector<UMPS::MessageFormats::DataPacket<T>> &&packets)
{
    checkPackets(packets);
    pImpl->mPackets = std::move(packets);
    pImpl->sortPackets();
}

template<class T>
int DataResponse<T>::getNumberOfPackets() const noexcept
{
    return static_cast<int> (pImpl->mPackets.size());
}

template<class T>
std::vector<UMPS::MessageFormats::DataPacket<T>>
    DataResponse<T>::getPackets() const noexcept
{
    return pImpl->mPackets;
}

template<class T>
const UMPS::MessageFormats::DataPacket<T>
    *DataResponse<T>::getPacketsPointer() const noexcept
{
    return pImpl->mPackets.data();
}

/// Identifier
template<class T>
void DataResponse<T>::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

template<class T>
uint64_t DataResponse<T>::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
template<class T>
void DataResponse<T>::setReturnCode(const ReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
}

template<class T>
ReturnCode DataResponse<T>::getReturnCode() const noexcept
{
    return pImpl->mReturnCode;
}

/// Message type
template<class T>
std::string DataResponse<T>::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Create CBOR
template<class T>
std::string DataResponse<T>::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From CBOR
template<class T>
void DataResponse<T>::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

template<class T>
void DataResponse<T>::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage<T>(data, length);
}

///  Convert message
template<class T>
std::string DataResponse<T>::toMessage() const
{
    return toCBOR();
}

template<class T>
void DataResponse<T>::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
template<class T>
std::unique_ptr<UMPS::MessageFormats::IMessage> DataResponse<T>::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<DataResponse<T>> (*this);
    return result;
}

/// Create an instance of this class 
template<class T>
std::unique_ptr<UMPS::MessageFormats::IMessage>
    DataResponse<T>::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<DataResponse<T>> (); 
    return result;
}

///--------------------------------------------------------------------------///
///                              Template Instantiation                      ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyServices::PacketCache::DataResponse<double>;
template class UMPS::ProxyServices::PacketCache::DataResponse<float>;
