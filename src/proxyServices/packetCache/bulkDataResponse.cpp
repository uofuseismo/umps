#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/packetCache/bulkDataResponse.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "private/applications/packetCache.hpp"

#define MESSAGE_TYPE "UMPS::ProxyServices::PacketCache::BulkDataResponse"

using namespace UMPS::ProxyServices::PacketCache;

namespace
{

template<class T>
nlohmann::json toJSONObject(
    const BulkDataResponse<T> &response)
{
    nlohmann::json obj;
    obj["MessageType"] = response.getMessageType();
    auto nDataResponses = response.getNumberOfDataResponses();
    obj["NumberOfDataResponses"] = nDataResponses;
    if (nDataResponses > 0)
    {
        nlohmann::json dataObjects;
        auto responsePtr = response.getDataResponsesPointer();
        for (int i = 0; i < nDataResponses; ++i)
        {
            nlohmann::json dataObject;
            auto nPackets = responsePtr[i].getNumberOfPackets();
            dataObject["NumberOfPackets"] = nPackets;
            auto packetsPointer = responsePtr[i].getPacketsPointer();
            if (nPackets > 0)
            {
                dataObject["Network"] = packetsPointer[0].getNetwork();
                dataObject["Station"] = packetsPointer[0].getStation();
                dataObject["Channel"] = packetsPointer[0].getChannel();
                dataObject["LocationCode"] = packetsPointer[0].getLocationCode();
                // Now the packets (these were sorted on time)
                nlohmann::json packetObjects;
                for (int ip = 0; ip < nPackets; ++ip)
                {
                    nlohmann::json packetObject;
                    packetObject["StartTime"]
                        = packetsPointer[ip].getStartTime().count();
                    packetObject["SamplingRate"]
                        = packetsPointer[ip].getSamplingRate();
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
                } // Loop on packets
                // Add the packets to this response object
                dataObject["Packets"] = packetObjects;
            } 
            else // No packets
            {
                dataObject["Packets"] = nullptr;
            } // End check on packets > 0
            dataObject["Identifier"] = responsePtr[i].getIdentifier();
            dataObject["ReturnCode"]
                = static_cast<int> (responsePtr[i].getReturnCode());
            // Update the data responses with the object for this request
            dataObjects.push_back(dataObject);
        } // Loop on responses
        // Finally save all objects for all requests 
        obj["DataResponses"] = dataObjects;
    }
    else
    {
        obj["DataResponses"] = nullptr;
    }
    obj["Identifier"] = response.getIdentifier();
    obj["ReturnCode"] = static_cast<int> (response.getReturnCode());
    return obj;
}

template<typename T>
BulkDataResponse<T> objectToBulkDataResponse(const nlohmann::json &obj)
{
    BulkDataResponse<T> response;
    if (obj["MessageType"] != response.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    auto nDataResponses = obj["NumberOfDataResponses"].get<int> ();
    if (nDataResponses > 0)
    {
        auto dataObjects = obj["DataResponses"];
        for (const auto &dataObject : dataObjects)
        {
            auto nPackets = dataObject["NumberOfPackets"].get<int> ();
            if (nPackets > 0)
            {
                DataResponse<T> dataResponse;
                auto network = dataObject["Network"].get<std::string> ();
                auto station = dataObject["Station"].get<std::string> ();
                auto channel = dataObject["Channel"].get<std::string> ();
                auto locationCode
                    = dataObject["LocationCode"].get<std::string> ();
                std::vector<UMPS::MessageFormats::DataPacket<T>> packets;
                auto packetObjects = dataObject["Packets"];
                packets.reserve(nPackets);
                for (const auto &packetObject : packetObjects)
                {
                    UMPS::MessageFormats::DataPacket<T> packet;
                    packet.setNetwork(network);
                    packet.setStation(station);
                    packet.setChannel(channel);
                    packet.setLocationCode(locationCode);
                    packet.setSamplingRate(packetObject["SamplingRate"]
                                          .get<double> ());
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
                if (!packets.empty()){dataResponse.setPackets(packets);}
                dataResponse.setIdentifier(dataObject["Identifier"]
                                          .get<uint64_t> ());
                auto rc = static_cast<ReturnCode>
                          (dataObject["ReturnCode"].get<int> ());
                dataResponse.setReturnCode(rc);
                response.addDataResponse(std::move(dataResponse));
            }
        }
    }
    response.setIdentifier(obj["Identifier"].get<uint64_t> ());
    response.setReturnCode(static_cast<ReturnCode>
                           (obj["ReturnCode"].get<int> ()));
    return response;
}

template<typename T>
BulkDataResponse<T> fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToBulkDataResponse<T>(obj);
}

template<typename T>
BulkDataResponse<T> fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToBulkDataResponse<T>(obj);
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
class BulkDataResponse<T>::BulkDataResponseImpl
{
public:
    std::vector<DataResponse<T>> mResponses;
    uint64_t mIdentifier = 0;
    ReturnCode mReturnCode = ReturnCode::SUCCESS;
};

/// C'tor
template<class T>
BulkDataResponse<T>::BulkDataResponse() :
    pImpl(std::make_unique<BulkDataResponseImpl> ())
{
}

/// Copy c'tor
template<class T>
BulkDataResponse<T>::BulkDataResponse(const BulkDataResponse<T> &response)
{
    *this = response;
}

/// Move c'tor
template<class T>
BulkDataResponse<T>::BulkDataResponse(BulkDataResponse<T> &&response) noexcept
{
    *this = std::move(response);
}

/// Copy assignment
template<class T>
BulkDataResponse<T>&
    BulkDataResponse<T>::operator=(const BulkDataResponse<T> &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<BulkDataResponseImpl> (*response.pImpl);
    return *this;
}

/// Move assignment
template<class T>
BulkDataResponse<T>&
    BulkDataResponse<T>::operator=(BulkDataResponse<T> &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

/// Destructor
template<class T>
BulkDataResponse<T>::~BulkDataResponse() = default;

/// Reset class
template<class T>
void BulkDataResponse<T>::clear() noexcept
{
    pImpl = std::make_unique<BulkDataResponseImpl> ();
}

/// Data responses
template<class T>
void BulkDataResponse<T>::addDataResponse(const DataResponse<T> &response)
{
    pImpl->mResponses.push_back(response);
}

template<class T>
void BulkDataResponse<T>::addDataResponse(DataResponse<T> &&response)
{
    pImpl->mResponses.push_back(std::move(response));
}

template<class T>
std::vector<DataResponse<T>>
    BulkDataResponse<T>::getDataResponses() const noexcept
{
    return pImpl->mResponses;
}

template<class T>
const DataResponse<T> 
    *BulkDataResponse<T>::getDataResponsesPointer() const noexcept
{
    return pImpl->mResponses.data();
}

template<class T>
int BulkDataResponse<T>::getNumberOfDataResponses() const noexcept
{
    return static_cast<int> (pImpl->mResponses.size());
}

/// Identifier
template<class T>
void BulkDataResponse<T>::setIdentifier(const uint64_t identifier) noexcept
{
    pImpl->mIdentifier = identifier;
}

template<class T>
uint64_t BulkDataResponse<T>::getIdentifier() const noexcept
{
    return pImpl->mIdentifier;
}

/// Return code
template<class T>
void BulkDataResponse<T>::setReturnCode(const ReturnCode code) noexcept
{
    pImpl->mReturnCode = code;
}

template<class T>
ReturnCode BulkDataResponse<T>::getReturnCode() const noexcept
{
    return pImpl->mReturnCode;
}

/// Message type
template<class T>
std::string BulkDataResponse<T>::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Create JSON
template<class T>
std::string BulkDataResponse<T>::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
template<class T>
std::string BulkDataResponse<T>::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result;
}

/// From JSON
template<class T>
void BulkDataResponse<T>::fromJSON(const std::string &message)
{
    *this = fromJSONMessage<T>(message);
}

/// From CBOR
template<class T>
void BulkDataResponse<T>::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

template<class T>
void BulkDataResponse<T>::fromCBOR(const uint8_t *data, const size_t length)
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
std::string BulkDataResponse<T>::toMessage() const
{
    return toCBOR();
}

template<class T>
void BulkDataResponse<T>::fromMessage(const char *messageIn,
                                      const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
template<class T>
std::unique_ptr<UMPS::MessageFormats::IMessage>
    BulkDataResponse<T>::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<BulkDataResponse<T>> (*this);
    return result;
}

/// Create an instance of this class 
template<class T>
std::unique_ptr<UMPS::MessageFormats::IMessage>
    BulkDataResponse<T>::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<BulkDataResponse<T>> (); 
    return result;
}

///--------------------------------------------------------------------------///
///                              Template Instantiation                      ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyServices::PacketCache::BulkDataResponse<double>;
template class UMPS::ProxyServices::PacketCache::BulkDataResponse<float>;
