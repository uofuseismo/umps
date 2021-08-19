#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <nlohmann/json.hpp>
#include "messageFormats/dataPacket.hpp"
#include "messageFormats/earthworm/traceBuf2.hpp"

using namespace URTS::MessageFormats;

namespace
{

bool isEmpty(const std::string &s) 
{
    if (s.empty()){return true;}
    return std::all_of(s.begin(), s.end(), [](const char c)
                       {
                           return std::isspace(c);
                       });
}

template<class T>
nlohmann::json toJSONObject(const DataPacket<T> &packet)
{
    nlohmann::json obj;
    obj["MessageType"] = packet.getMessageType();
    obj["Network"] = packet.getNetwork();
    obj["Station"] = packet.getStation();
    obj["Channel"] = packet.getChannel();
    obj["LocationCode"] = packet.getLocationCode();
    obj["StartTime"] = packet.getStartTime();
    obj["SamplingRate"] = packet.getSamplingRate();
    if (packet.haveSamplingRate() && packet.getNumberOfSamples() > 0)
    {
        obj["EndTime"] = packet.getEndTime();
    }
    else
    {
        obj["EndTime"] = nullptr;
    }
    if (packet.getNumberOfSamples() > 0)
    {
        obj["Data"] = packet.getData();
    }
    else
    {
        obj["Data"] = nullptr;
    }
    return obj;
}

template<class T>
void objectToDataPacket(const nlohmann::json &obj, DataPacket<T> *packet)
{
    packet->clear();
    if (obj["MessageType"] != packet->getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Essential stuff
    packet->setNetwork(obj["Network"].get<std::string> ());
    packet->setStation(obj["Station"].get<std::string> ());
    packet->setChannel(obj["Channel"].get<std::string> ());
    packet->setLocationCode(obj["LocationCode"].get<std::string> ());
    packet->setSamplingRate(obj["SamplingRate"].get<double> ());
    packet->setStartTime(obj["StartTime"].get<int64_t> ());
    std::vector<T> data = obj["Data"]; //.get<std::vector<T>> ());   
    if (!data.empty()){packet->setData(std::move(data));}
}



template<class T>
void fromJSONMessage(const std::string &message, DataPacket<T> *packet)
{
    auto obj = nlohmann::json::parse(message);
    objectToDataPacket(obj, packet);
}

template<class T>
void fromCBORMessage(const uint8_t *message, const size_t length,
                     DataPacket<T> *packet)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    objectToDataPacket(obj, packet);
}

}

template<class T>
class DataPacket<T>::DataPacketImpl
{
public:
    void updateEndTime()
    {
        mEndTimeMicroSeconds = mStartTimeMicroSeconds;
        if (!mData.empty() && mSamplingRate > 0)
        {
            auto nSamples = static_cast<int> (mData.size());
            auto traceDuration
                = std::round( ((nSamples - 1)/mSamplingRate)*1000000 );
            mEndTimeMicroSeconds = mStartTimeMicroSeconds
                                 + static_cast<int64_t> (traceDuration);
        }
    }
    std::vector<T> mData;
    std::string mNetwork;
    std::string mStation;
    std::string mChannel;
    std::string mLocationCode;
    double mSamplingRate = 0;
    /// Start time in microseconds (10e-6)
    int64_t mStartTimeMicroSeconds = 0;
    /// End time in microseconds (10e-6)
    int64_t mEndTimeMicroSeconds = 0;
};

/// Clear class
template<class T>
void DataPacket<T>::clear() noexcept
{
    pImpl->mData.clear();
    pImpl->mNetwork.clear();
    pImpl->mStation.clear();
    pImpl->mChannel.clear();
    pImpl->mLocationCode.clear();
    pImpl->mSamplingRate = 0;
    pImpl->mStartTimeMicroSeconds = 0;
}

/// C'tor
template<class T>
DataPacket<T>::DataPacket() :
    pImpl(std::make_unique<DataPacketImpl> ())
{
}

/// Copy c'tor 
template<class T>
DataPacket<T>::DataPacket(const DataPacket<T> &packet)
{
    *this = packet;
}

/*
/// Copy c'tor
template<class T>
DataPacket<T>::DataPacket(const Earthworm::TraceBuf2<T> &traceBuf)
{
    *this = traceBuf;
}
*/

/// Move c'tor
template<class T>
DataPacket<T>::DataPacket(DataPacket<T> &&packet) noexcept
{
    *this = std::move(packet);
}

/// Copy assignment
template<class T>
DataPacket<T>& DataPacket<T>::operator=(const DataPacket<T> &packet)
{
    if (&packet == this){return *this;}
    pImpl = std::make_unique<DataPacketImpl> (*packet.pImpl);
    return *this;
}

/// Move assignment
template<class T>
DataPacket<T>& DataPacket<T>::operator=(DataPacket<T> &&packet) noexcept
{
    if (&packet == this){return *this;}
    pImpl = std::move(packet.pImpl);
    return *this;
}

/// Destructor
template<class T>
DataPacket<T>::~DataPacket() = default;

/*
/// Convert a tracebuf2 to a datapacket
template<class T>
template<typename U>
void DataPacket<T>::fromTraceBuf2(const Earthworm::TraceBuf2<U> &traceBuf)
{
    setNetwork(traceBuf.getNetwork());
    setStation(traceBuf.getStation());
    setChannel(traceBuf.getChannel());
    setLocationCode(traceBuf.getLocationCode());
    if (traceBuf.haveSamplingRate())
    {
        setSamplingRate(traceBuf.getSamplingRate());
    }
    auto startTime = traceBuf.getStartTime();
    auto startTimeMuS = static_cast<int64_t> (std::round(startTime*1.e9));
    setStartTime(startTimeMuS);
    auto nSamples = traceBuf.getNumberOfSamples();
    if (nSamples > 0){setData(nSamples, traceBuf.getDataPointer());}
}
*/

/// Network
template<class T>
void DataPacket<T>::setNetwork(const std::string &network)
{
    if (isEmpty(network)){throw std::invalid_argument("Network is empty");}
    pImpl->mNetwork = network;
}

template<class T>
std::string DataPacket<T>::getNetwork() const
{
    if (!haveNetwork()){throw std::runtime_error("Network not set yet");}
    return pImpl->mNetwork;
}

template<class T>
bool DataPacket<T>::haveNetwork() const noexcept
{
    return !pImpl->mNetwork.empty();
}

/// Station
template<class T>
void DataPacket<T>::setStation(const std::string &station)
{
    if (isEmpty(station)){throw std::invalid_argument("Station is empty");}
    pImpl->mStation = station;
}

template<class T>
std::string DataPacket<T>::getStation() const
{
    if (!haveStation()){throw std::runtime_error("Station not set yet");}
    return pImpl->mStation;
}

template<class T>
bool DataPacket<T>::haveStation() const noexcept
{
    return !pImpl->mStation.empty();
}

/// Channel
template<class T>
void DataPacket<T>::setChannel(const std::string &channel)
{
    if (isEmpty(channel)){throw std::invalid_argument("Channel is empty");}
    pImpl->mChannel = channel;
}

template<class T>
std::string DataPacket<T>::getChannel() const
{
    if (!haveChannel()){throw std::runtime_error("Channel not set yet");}
    return pImpl->mChannel;
}

template<class T>
bool DataPacket<T>::haveChannel() const noexcept
{
    return !pImpl->mChannel.empty();
}

/// Location code
template<class T>
void DataPacket<T>::setLocationCode(const std::string &location)
{
    if (isEmpty(location)){throw std::invalid_argument("Location is empty");}
    pImpl->mLocationCode = location;
}

template<class T>
std::string DataPacket<T>::getLocationCode() const
{
    if (!haveLocationCode())
    {
        throw std::runtime_error("Location code not set yet");
    }
    return pImpl->mLocationCode;
}

template<class T>
bool DataPacket<T>::haveLocationCode() const noexcept
{
    return !pImpl->mLocationCode.empty();
}

/// Sampling rate
template<class T>
void DataPacket<T>::setSamplingRate(const double samplingRate) 
{
    if (samplingRate <= 0)
    {
        throw std::invalid_argument("samplingRate = "
                                  + std::to_string(samplingRate)
                                  + " must be positive");
    }
    pImpl->mSamplingRate = samplingRate;
    pImpl->updateEndTime();
}

template<class T>
double DataPacket<T>::getSamplingRate() const
{
    if (!haveSamplingRate()){throw std::runtime_error("Sampling rate not set");}
    return pImpl->mSamplingRate;
}

template<class T>
bool DataPacket<T>::haveSamplingRate() const noexcept
{
    return (pImpl->mSamplingRate > 0);     
}

/// Number of samples
template<class T>
int DataPacket<T>::getNumberOfSamples() const noexcept
{
    return static_cast<int> (pImpl->mData.size());
}

/// Start time
template<class T>
void DataPacket<T>::setStartTime(const int64_t startTime) noexcept
{
    pImpl->mStartTimeMicroSeconds = startTime;
    pImpl->updateEndTime();
}

template<class T>
int64_t DataPacket<T>::getStartTime() const noexcept
{
    return pImpl->mStartTimeMicroSeconds;
}

template<class T>
int64_t DataPacket<T>::getEndTime() const
{
    if (!haveSamplingRate())
    {   
        throw std::runtime_error("Sampling rate note set");
    }   
    if (getNumberOfSamples() < 1)
    {   
        throw std::runtime_error("No samples in signal");
    }   
    return pImpl->mEndTimeMicroSeconds;
}

/// Sets the data
template<class T>
void DataPacket<T>::setData(std::vector<T> &&x) noexcept
{
    pImpl->mData = std::move(x);
    pImpl->updateEndTime();
}

template<class T>
template<typename U>
void DataPacket<T>::setData(const std::vector<U> &x) noexcept
{
    setData(x.size(), x.data());
}

template<class T>
template<typename U>
void DataPacket<T>::setData(const int nSamples, const U *__restrict__ x)
{
    // Invalid
    if (nSamples < 0)
    {   
        throw std::invalid_argument("nSamples not positive");
    }   
    // No data so nothing to do
    pImpl->mData.resize(nSamples);
    pImpl->updateEndTime();
    if (nSamples == 0){return;}
    if (x == nullptr){throw std::invalid_argument("x is NULL");}
    T *__restrict__ dPtr = pImpl->mData.data(); 
    std::copy(x, x + nSamples, dPtr);
}

/// Gets the data
template<class T>
std::vector<T> DataPacket<T>::getData() const noexcept
{
    return pImpl->mData;
}

/// Message format
template<class T>
std::string DataPacket<T>::getMessageType() const noexcept
{
    return "DataPacket";
}

/// Copy this class
template<class T>
std::unique_ptr<URTS::MessageFormats::IMessage> 
    DataPacket<T>::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::DataPacket<T>> (*this);
    return result;
}

/// Create an instance of this class 
template<class T>
std::unique_ptr<URTS::MessageFormats::IMessage>
    DataPacket<T>::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::DataPacket<T>> (); 
    return result;
}

/// From CBOR
template<class T>
void DataPacket<T>::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

template<class T>
void DataPacket<T>::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    DataPacket<T> packet;
    fromCBORMessage(data, length, &packet);
    *this = std::move(packet);
}

/// To CBOR
template<class T>
std::string DataPacket<T>::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto message = nlohmann::json::to_cbor(obj);
    std::string result(message.begin(), message.end());
    return result;
}

/// To JSON
template<class T>
std::string DataPacket<T>::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    auto result = obj.dump(nIndent);
    return result;
}

///--------------------------------------------------------------------------///
///                            Template Instantiation                        ///
///--------------------------------------------------------------------------///
template class URTS::MessageFormats::DataPacket<double>;
template class URTS::MessageFormats::DataPacket<float>;

template void URTS::MessageFormats::DataPacket<double>::setData(const std::vector<double> &x);
template void URTS::MessageFormats::DataPacket<double>::setData(const std::vector<float> &x);
template void URTS::MessageFormats::DataPacket<double>::setData(const std::vector<int> &x);
template void URTS::MessageFormats::DataPacket<double>::setData(const std::vector<int16_t> &data);
template void URTS::MessageFormats::DataPacket<double>::setData(const int nSamples, const double *data);
template void URTS::MessageFormats::DataPacket<double>::setData(const int nSamples, const float *data);
template void URTS::MessageFormats::DataPacket<double>::setData(const int nSamples, const int *data);
template void URTS::MessageFormats::DataPacket<double>::setData(const int nSamples, const int16_t *data);

template void URTS::MessageFormats::DataPacket<float>::setData(const std::vector<double> &x);
template void URTS::MessageFormats::DataPacket<float>::setData(const std::vector<float> &x);
template void URTS::MessageFormats::DataPacket<float>::setData(const std::vector<int> &x); 
template void URTS::MessageFormats::DataPacket<float>::setData(const std::vector<int16_t> &data);
template void URTS::MessageFormats::DataPacket<float>::setData(const int nSamples, const double *data);
template void URTS::MessageFormats::DataPacket<float>::setData(const int nSamples, const float *data);
template void URTS::MessageFormats::DataPacket<float>::setData(const int nSamples, const int *data);
template void URTS::MessageFormats::DataPacket<float>::setData(const int nSamples, const int16_t *data);

/*
template void URTS::MessageFormats::DataPacket<double>::fromTraceBuf2(const Earthworm::TraceBuf2<double> &tb);
template void URTS::MessageFormats::DataPacket<double>::fromTraceBuf2(const Earthworm::TraceBuf2<float> &tb);
template void URTS::MessageFormats::DataPacket<double>::fromTraceBuf2(const Earthworm::TraceBuf2<int> &tb);
template void URTS::MessageFormats::DataPacket<double>::fromTraceBuf2(const Earthworm::TraceBuf2<int16_t> &tb);
template void URTS::MessageFormats::DataPacket<float>::fromTraceBuf2(const Earthworm::TraceBuf2<double> &tb);
template void URTS::MessageFormats::DataPacket<float>::fromTraceBuf2(const Earthworm::TraceBuf2<float> &tb);
template void URTS::MessageFormats::DataPacket<float>::fromTraceBuf2(const Earthworm::TraceBuf2<int> &tb);
template void URTS::MessageFormats::DataPacket<float>::fromTraceBuf2(const Earthworm::TraceBuf2<int16_t> &tb);
*/
