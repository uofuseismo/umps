#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <string>
#include <cassert>
#include <bit>
//#include <boost/json/src.hpp>
#include <nlohmann/json.hpp>
#include "umps/messageFormats/earthworm/traceBuf2.hpp"
#ifdef WITH_EARTHWORM
   #include "trace_buf.h"
   #define MAX_TRACE_SIZE (MAX_TRACEBUF_SIZ - 64)
   #define STA_LEN (TRACE2_STA_LEN  - 1)
   #define NET_LEN (TRACE2_NET_LEN  - 1)
   #define CHA_LEN (TRACE2_CHAN_LEN - 1)
   #define LOC_LEN (TRACE2_LOC_LEN  - 1)
#else
   // These values are from Earthworm's trace_buf.h but subtracted by 1
   // since std::string will handle the NULL termination for us.
   #define MAX_TRACE_SIZE 4032 //4096 - 64 
   #define STA_LEN 6
   #define NET_LEN 8
   #define CHA_LEN 3
   #define LOC_LEN 2
#endif
 
#define MESSAGE_TYPE "UMPS::MessageFormats::Earthworm::TraceBuf2"

using namespace UMPS::MessageFormats::Earthworm;

namespace
{
/// Copies an input network/station/channel/location code while respecting
/// the max size for the parameter.
void copyString(std::string *result,
                const std::string &sIn, const int outputLength)
{
    auto nCopy = std::min(sIn.size(), static_cast<size_t> (outputLength));
    result->resize(nCopy);
    std::copy(sIn.c_str(), sIn.c_str() + nCopy, result->begin());
}

/// Utility function to define the appropriate format
template<typename T> 
std::string getDataFormat() noexcept
{
    std::string result(2, '\0'); 
    if constexpr (std::endian::native == std::endian::little) 
    {
        if constexpr (std::is_same<T, double>::value)
        {
            result = "f8";
        }
        else if constexpr (std::is_same<T, float>::value)
        {
            result = "f4";
        }
        else if constexpr (std::is_same<T, int>::value)
        {
            result = "i4";
        }
        else if constexpr (std::is_same<T, int16_t>::value)
        {
            result = "i2";
        }
        else
        {
            std::cerr << "Unhandled little endian precision" << std::endl;
#ifndef NDEBUG
            assert(false);
#endif
        }
    }
    else if constexpr (std::endian::native == std::endian::big)
    {
        if constexpr (std::is_same<T, double>::value)
        {   
            result = "t8";
        }   
        else if constexpr (std::is_same<T, float>::value)
        {   
            result = "t4";
        }
        else if constexpr (std::is_same<T, int>::value)
        {   
            result = "s4";
        }   
        else if constexpr (std::is_same<T, int16_t>::value)
        {   
            result = "s2";
        }
        else
        {
            std::cerr << "Unhandled big endian precision" << std::endl;
#ifndef NDEBUG
            assert(false);
#endif
        }
    }
    else
    {
        std::cerr << "Mixed endianness is unhandled" << std::endl;
#ifndef NDEBUG
        assert(false);
#endif
    } 
    return result;
}

/// Utility function to define the correct message length
template<typename T>  
int getMaxTraceLength() noexcept
{
#ifndef NDEBUG
    assert(MAX_TRACE_SIZE%sizeof(T) == 0);
#endif
    return MAX_TRACE_SIZE/sizeof(T);
}

/// Unpacks data
template<typename T> T unpack(const char *__restrict__ cIn,
                              const bool swap = false)
{
    union
    {
        char c[sizeof(T)];
        T value;
    };
    if (!swap)
    {
        std::copy(cIn, cIn + sizeof(T), c);
    }
    else
    {
        std::reverse_copy(cIn, cIn + sizeof(T), c);
    }
    return value;
}

template<typename T, typename U> std::vector<T> 
unpack(const char *__restrict__ cIn, const int nSamples, const bool swap)
{
    std::vector<T> result(nSamples);
    if (!swap)
    {
        auto dPtr = reinterpret_cast<const U *__restrict__> (cIn);
        std::copy(dPtr, dPtr + nSamples, result.data());
    }
    else
    {
        const auto nBytes = sizeof(U);
        auto resultPtr = result.data();
        for (int i = 0; i < nSamples; ++i)
        {
            resultPtr[i] = static_cast<T> (unpack<U>(cIn + i*nBytes, swap));
        }
    }
    return result;
}

template<typename T>
TraceBuf2<T> unpackEarthwormMessage(const char *message)
{
    // Bytes  0 - 3:  pinno (int)
    // Bytes  4 - 7:  nsamp (int)
    // Bytes  8 - 15: starttime (double)
    // Bytes 16 - 23: endtime (double)
    // Bytes 24 - 31: sampling rate (double)
    // Bytes 32 - 38: station (char)
    // Bytes 39 - 44: network (char)
    // Bytes 48 - 51: channel (char)
    // Bytes 52 - 54: location (char)
    // Bytes 55 - 56: version (char)
    // Bytes 57 - 59: datatype (char) 
    // Bytes 60 - 61: quality (char)
    // Bytes 62 - 63: pad (char) 
    TraceBuf2<T> result;
    // First figure out the data format (int, double, float, etc.)
    bool swap = false;
    char dtype = 'i';
    if (message[57] == 'i')
    {
        if constexpr (std::endian::native == std::endian::big){swap = true;}
        dtype = 'i';
    }
    else if (message[57] == 'f')
    {
        if constexpr (std::endian::native == std::endian::big){swap = true;} 
        dtype = 'f';
    }
    else if (message[57] == 's')
    {
        if constexpr (std::endian::native == std::endian::little){swap = true;}
        dtype = 'i';
    }
    else if (message[57] == 't')
    {
        if constexpr (std::endian::native == std::endian::little){swap = true;}
        dtype = 'f';
    }
    // Now figure out the number of bytes
    int nBytes = 4;
    if (message[58] == '4')
    {
        nBytes = 4;
    }
    else if (message[58] == '2')
    {
        nBytes = 2;
        if (dtype == 'f')
        {
            std::cerr << "float16 unhandled" << std::endl;
#ifndef NDEBUG
            assert(false);
#endif
            return result; 
        }
    }
    else if (message[58] == '8')
    {
        nBytes = 8;
    }
    else
    {
        std::cerr << "Unhandled number of bytes" << std::endl;
#ifndef NDEBUG
        assert(false);
#endif
        return result;
    }
    // Unpack some character info
    std::string station(message + 32);
    std::string network(message + 39);
    std::string channel(message + 48);
    std::string location(message + 52); 
    result.setNetwork(network);
    result.setStation(station);
    result.setChannel(channel);
    result.setLocationCode(location);
    // Finally unpack the data
    if (!swap)
    {
        constexpr bool swapPass = false;
        auto pinno        = unpack<int>(&message[0],      swapPass);
        auto nsamp        = unpack<int>(&message[4],      swapPass);
        auto startTime    = unpack<double>(&message[8],   swapPass);
        //auto endTime    = unpack<double>(&message[16],  swapPass);
        auto samplingRate = unpack<double>(&message[24],  swapPass);
        auto quality      = unpack<int16_t>(&message[60], swapPass);
        result.setPinNumber(pinno);
        result.setStartTime(startTime);
        result.setSamplingRate(samplingRate);
        result.setQuality(quality);

        if (dtype == 'i')
        {
            if (nBytes == 2)
            {
                auto dPtr = reinterpret_cast<const int16_t *> (message + 64);
                result.setData(nsamp, dPtr);
            }
            else if (nBytes == 4)
            {
                auto dPtr = reinterpret_cast<const int32_t *> (message + 64);
                result.setData(nsamp, dPtr);
            }
            else if (nBytes == 8)
            {
                auto dPtr = reinterpret_cast<const int64_t *> (message + 64);
                result.setData(nsamp, dPtr);
            }
        }
        else if (dtype == 'f' && nBytes == 4)
        {
            if (nBytes == 4)
            {
                auto dPtr = reinterpret_cast<const float *> (message + 64);
                result.setData(nsamp, dPtr);
            }
            else if (nBytes == 8)
            {
                auto dPtr = reinterpret_cast<const double *> (message + 64);
                result.setData(nsamp, dPtr);
            }
        }
        else
        {
           std::cerr << "Can only process i or f datatype" << std::endl;
#ifndef NDEBUG
           assert(false);
#endif
        }
    }
    else
    {
        constexpr bool swapPass = true;
        auto pinno        = unpack<int>(&message[0],      swapPass);
        auto nsamp        = unpack<int>(&message[4],      swapPass);
        auto startTime    = unpack<double>(&message[8],   swapPass);
        auto samplingRate = unpack<double>(&message[24],  swapPass);
        auto quality      = unpack<int16_t>(&message[60], swapPass);
        result.setPinNumber(pinno);
        result.setStartTime(startTime);
        result.setSamplingRate(samplingRate);
        result.setQuality(quality);

        std::vector<T> x;
        if (dtype == 'i')
        {
            if (nBytes == 2)
            {
                x = unpack<T, int16_t>(message + 64, nsamp, swapPass);
            }
            else if (nBytes == 4)
            {
                x = unpack<T, int32_t>(message + 64, nsamp, swapPass);
            }
            else if (nBytes == 8)
            {
                x = unpack<T, int64_t>(message + 64, nsamp, swapPass);
            }
        }
        else if (dtype == 'f' && nBytes == 4)
        {
            if (nBytes == 4)
            {
                x = unpack<T, float>(message + 64, nsamp, swapPass);
            }
            else if (nBytes == 8)
            {
                x = unpack<T, double>(message + 64, nsamp, swapPass);
            }
        }
        else
        {
           std::cerr << "Can only process i or f datatype" << std::endl;
#ifndef NDEBUG
           assert(false);
#endif
        }
        result.setData(std::move(x));
    }
    return result; 
}

template<class T>
nlohmann::json toJSONObject(const TraceBuf2<T> &tb)
{
    nlohmann::json obj;
    obj["MessageType"] = tb.getMessageType();
    obj["Network"] = tb.getNetwork();
    obj["Station"] = tb.getStation();
    obj["Channel"] = tb.getChannel();
    obj["LocationCode"] = tb.getLocationCode();
    obj["StartTime"] = tb.getStartTime();
    obj["SamplingRate"] = tb.getSamplingRate();
    obj["PinNumber"] = tb.getPinNumber();
    obj["Quality"] = tb.getQuality();
    obj["Version"] = tb.getVersion();
    if (tb.haveSamplingRate() && tb.getNumberOfSamples() > 0)
    {   
        obj["EndTime"] = tb.getEndTime();
    }
    else
    {
        obj["EndTime"] = nullptr;
    }
    if (tb.getNumberOfSamples() > 0)
    {
        obj["Data"] = tb.getData();
    }
    else
    {
        obj["Data"] = nullptr;
    }
    return obj;
}

template<typename T>
void objectToTraceBuf2(const nlohmann::json obj,
                       TraceBuf2<T> *tb)
{
    tb->clear();
    auto messageType = obj["MessageType"];
    if (messageType != tb->getMessageType())
    {
        std::cerr << "Invalid message type: " << messageType << std::endl;
        return;
    }
    // Unpack the message
    std::string network = obj["Network"];
    std::string station = obj["Station"];
    std::string channel = obj["Channel"];
    std::string locationCode = obj["LocationCode"];
    double startTime = obj["StartTime"];
    double samplingRate = obj["SamplingRate"];
    int pinNumber = obj["PinNumber"];
    int quality = obj["Quality"];
    tb->setNetwork(network);
    tb->setStation(station);
    tb->setChannel(channel);
    tb->setLocationCode(locationCode);
    tb->setStartTime(startTime);
    tb->setSamplingRate(samplingRate);
    tb->setPinNumber(pinNumber);
    tb->setQuality(quality);
    std::vector<T> data = obj["Data"]; 
    if (!data.empty()){tb->setData(std::move(data));}
}

template<typename T>
void fromJSONMessage(const std::string &message,
                     TraceBuf2<T> *tb)
{
    auto obj = nlohmann::json::parse(message);
    return objectToTraceBuf2(obj, tb);
}

template<typename T>
void fromCBORMessage(const uint8_t *message, const size_t length,
                     TraceBuf2<T> *tb)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToTraceBuf2(obj, tb);
}

}

/// The implementation
template<class T>
class TraceBuf2<T>::TraceBuf2Impl
{
public:
    void updateEndTime()
    {
        mEndTime = mStartTime;
        if (!mData.empty() && mSamplingRate > 0)
        {
            mEndTime = mStartTime
                     + static_cast<double> (mData.size() - 1)/mSamplingRate;
        }
    }
    void clear() noexcept
    {
        mData.clear();
        mNetwork.clear();
        mStation.clear();
        mChannel.clear();
        mLocationCode.clear();
        mVersion = "20";
        mQuality = 0;//"\0\0";
        mStartTime = 0;
        mEndTime = 0;
        mSamplingRate = 0;
        mPinNumber = 0;
    }
    /// The data in the packet.
    std::vector<T> mData; 
    /// The network code.
    std::string mNetwork;//{NET_LEN, '\0'};
    /// The station code.
    std::string mStation;//{STA_LEN, '\0'};
    /// The channel code.
    std::string mChannel;//{CHA_LEN, '\0'};
    /// The location code.
    std::string mLocationCode;//{LOC_LEN, '-'};
    /// Default to version 2.0.
    std::string mVersion{"20"};
    /// The data format
    const std::string mDataType = getDataFormat<T>();
    /// The quality
    //std::string mQuality{2, '\0'}; // Default to no quality
    /// The UTC time of the first sample in seconds from the epoch.
    double mStartTime = 0;
    /// The UTC time of the last sample in seconds from the epoch.
    double mEndTime = 0;
    /// The sampling rate in Hz.
    double mSamplingRate = 0;
    /// The pin number.
    int mPinNumber = 0;
    /// Data quality
    int mQuality = 0;
    /// Max trace length
    const int mMaximumNumberOfSamples = getMaxTraceLength<T>();
};

/// C'tor
template<class T>
TraceBuf2<T>::TraceBuf2() :
    pImpl(std::make_unique<TraceBuf2Impl> ())
{
}

/// Copy c'tor
template<class T>
TraceBuf2<T>::TraceBuf2(const TraceBuf2<T> &traceBuf2)
{
    *this = traceBuf2;
}

/// Move c'tor
template<class T>
TraceBuf2<T>::TraceBuf2(TraceBuf2<T> &&traceBuf2) noexcept
{
    *this = std::move(traceBuf2);
}

/// Copy assignment
template<class T>
TraceBuf2<T> &TraceBuf2<T>::operator=(const TraceBuf2<T> &traceBuf2)
{
    if (&traceBuf2 == this){return *this;}
    pImpl = std::make_unique<TraceBuf2Impl> (*traceBuf2.pImpl);
    return *this;
}

/// Move assignment
template<class T>
TraceBuf2<T> &TraceBuf2<T>::operator=(TraceBuf2<T> &&traceBuf2) noexcept
{
    if (&traceBuf2 == this){return *this;}
    pImpl = std::move(traceBuf2.pImpl);
    return *this;
}

/// Set the network name
template<class T>
void TraceBuf2<T>::setNetwork(const std::string &network) noexcept
{
    copyString(&pImpl->mNetwork, network, getMaximumNetworkLength());
}

template<class T>
std::string TraceBuf2<T>::getNetwork() const noexcept
{
    return pImpl->mNetwork;
}

template<class T>
int TraceBuf2<T>::getMaximumNetworkLength() const noexcept
{
    return NET_LEN;
} 

/// Set the station name
template<class T>
void TraceBuf2<T>::setStation(const std::string &station) noexcept
{
    copyString(&pImpl->mStation, station, getMaximumStationLength());
}

template<class T>
std::string TraceBuf2<T>::getStation() const noexcept
{
    return pImpl->mStation;
}

template<class T>
int TraceBuf2<T>::getMaximumStationLength() const noexcept
{
    return STA_LEN;
}

/// Set the channel name
template<class T>
void TraceBuf2<T>::setChannel(const std::string &channel) noexcept
{
    copyString(&pImpl->mChannel, channel, getMaximumChannelLength());
}

template<class T>
std::string TraceBuf2<T>::getChannel() const noexcept
{
    return pImpl->mChannel;
}

template<class T>
int TraceBuf2<T>::getMaximumChannelLength() const noexcept
{
    return CHA_LEN;
}

/// Set the location code
template<class T>
void TraceBuf2<T>::setLocationCode(const std::string &location) noexcept
{
    copyString(&pImpl->mLocationCode, location, getMaximumLocationCodeLength());
}

template<class T>
std::string TraceBuf2<T>::getLocationCode() const noexcept
{
    return pImpl->mLocationCode;
}

template<class T>
int TraceBuf2<T>::getMaximumLocationCodeLength() const noexcept
{
    return LOC_LEN;
}

/// Set the start time
template<class T>
void TraceBuf2<T>::setStartTime(const double startTime) noexcept
{
    pImpl->mStartTime = startTime;
    pImpl->updateEndTime();
}

template<class T>
double TraceBuf2<T>::getStartTime() const noexcept
{
    return pImpl->mStartTime;
}

/// Get end time
template<class T>
double TraceBuf2<T>::getEndTime() const
{
    if (!haveSamplingRate())
    {
        throw std::runtime_error("Sampling rate note set");
    }
    if (getNumberOfSamples() < 1)
    {
        throw std::runtime_error("No samples in signal");
    }
    return pImpl->mEndTime;
}

/// Set the sampling rate
template<class T>
void TraceBuf2<T>::setSamplingRate(const double samplingRate)
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
double TraceBuf2<T>::getSamplingRate() const
{
    if (!haveSamplingRate()){throw std::runtime_error("Sampling rate not set");}
    return pImpl->mSamplingRate;
}

template<class T>
bool TraceBuf2<T>::haveSamplingRate() const noexcept
{
    return (pImpl->mSamplingRate > 0);
}

/// Set quality
template<class T>
void TraceBuf2<T>::setQuality(const int quality) noexcept 
{
    // Quality flags in header:
    // AMPLIFIER_SATURATED    0x01 (=1)
    // DIGITIZER_CLIPPED      0x02 (=2)
    // SPIKES_DETECTED        0x04 (=4)
    // GLITCHES_DETECTED      0x08 (=8)
    // MISSING_DATA_PRESENT   0x10 (=16)
    // TELEMETRY_SYNCH_ERROR  0x20 (=20)
    // FILTER_CHARGING        0x40 (=40)
    // TIME_TAG_QUESTIONABLE  0x80 (=80)
    pImpl->mQuality = quality;
}

template<class T>
int TraceBuf2<T>::getQuality() const noexcept
{
    return pImpl->mQuality;
}

/// Get number of samples
template<class T>
int TraceBuf2<T>::getNumberOfSamples() const noexcept
{
    return static_cast<int> (pImpl->mData.size());
}

/// Maximum number of samples
template<class T>
int TraceBuf2<T>::getMaximumNumberOfSamples() const noexcept
{
    return pImpl->mMaximumNumberOfSamples;
}

/// Set data
template<class T>
template<typename U>
void TraceBuf2<T>::setData(const std::vector<U> &x)
{
    setData(x.size(), x.data());
}

/// Set data
template<class T>
void TraceBuf2<T>::setData(std::vector<T> &&x) noexcept
{
    pImpl->mData = std::move(x); 
    pImpl->updateEndTime();
}

/// Set data
template<class T>
template<typename U>
void TraceBuf2<T>::setData(const int nSamples, const U *__restrict__ x)
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

/// Get data
template<class T>
std::vector<T> TraceBuf2<T>::getData() const noexcept
{
    return pImpl->mData;
}

template<class T>
const T *TraceBuf2<T>::getDataPointer() const
{
    if (pImpl->mData.empty()){throw std::runtime_error("No data set");}
    return pImpl->mData.data();
}

/// Version
template<class T>
std::string TraceBuf2<T>::getVersion() const noexcept
{
    return pImpl->mVersion;
}    

/// Pin number
template<class T>
void TraceBuf2<T>::setPinNumber(const int pinNumber) noexcept
{
    pImpl->mPinNumber = pinNumber;
}

template<class T>
int TraceBuf2<T>::getPinNumber() const noexcept
{
    return pImpl->mPinNumber;
}

/// Destructor
template<class T>
TraceBuf2<T>::~TraceBuf2() = default;

/// Reset class
template<class T>
void TraceBuf2<T>::clear() noexcept
{
    pImpl->clear();
}

/// Serialize
template<class T>
void TraceBuf2<T>::fromEarthworm(const char *message)
{
    auto t = unpackEarthwormMessage<T>(message);
    *this = std::move(t);
}

template<class T>
std::string TraceBuf2<T>::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    auto result = obj.dump(nIndent);
    return result;
/*
    //boost::json::object obj;
    nlohmann::json obj;
    obj["MessageType"] = getMessageType();
    obj["Network"] = getNetwork();
    obj["Station"] = getStation();
    obj["Channel"] = getChannel();
    obj["LocationCode"] = getLocationCode();
    obj["StartTime"] = getStartTime();
    obj["SamplingRate"] = getSamplingRate();
    obj["PinNumber"] = getPinNumber();
    obj["Quality"] = getQuality();
    obj["Version"] = getVersion();
    if (haveSamplingRate() && getNumberOfSamples() > 0)
    {
        obj["EndTime"] = getEndTime();
    }
    else
    {
        obj["EndTime"] = nullptr;
    }
    if (getNumberOfSamples() > 0)
    {
        //obj["Data"] = boost::json::value_from(getData());
        obj["Data"] = getData();
    }
    else
    {
        obj["Data"] = nullptr;
    }
    // Make it a string
    //auto result = boost::json::serialize(obj);
    auto result = obj.dump(nIndent);
std::cout << result.size() << std::endl;
    auto msgPack = nlohmann::json::to_cbor(obj);
std::cout << msgPack.size() << std::endl;
    return result;
*/
}

template<class T>
std::string TraceBuf2<T>::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto message = nlohmann::json::to_cbor(obj);
    std::string result(message.begin(), message.end());
    return result;
}

template<class T>
void TraceBuf2<T>::fromJSON(const std::string &message) 
{
    TraceBuf2<T> tb;
    fromJSONMessage(message, &tb);
    *this = std::move(tb);
}

/// Copy this class
template<class T>
std::unique_ptr<UMPS::MessageFormats::IMessage>
    TraceBuf2<T>::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Earthworm::TraceBuf2<T>> (*this);
    return result;
}

/// Create an instance of this class 
template<class T>
std::unique_ptr<UMPS::MessageFormats::IMessage> 
    TraceBuf2<T>::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Earthworm::TraceBuf2<T>> ();
    return result;
}

/// From CBOR
template<class T>
void TraceBuf2<T>::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

template<class T>
void TraceBuf2<T>::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    TraceBuf2<T> tb;
    fromCBORMessage(data, length, &tb);
    *this = std::move(tb);
}

/// Message type
template<class T>
std::string TraceBuf2<T>::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Swap 
void UMPS::MessageFormats::Earthworm::swap(TraceBuf2<double> &lhs,
                                           TraceBuf2<double> &rhs)
{
    std::swap(lhs.pImpl, rhs.pImpl);
}

void UMPS::MessageFormats::Earthworm::swap(TraceBuf2<float> &lhs,
                                           TraceBuf2<float> &rhs)
{
    std::swap(lhs.pImpl, rhs.pImpl);
}

void UMPS::MessageFormats::Earthworm::swap(TraceBuf2<int> &lhs,
                                           TraceBuf2<int> &rhs)
{
    std::swap(lhs.pImpl, rhs.pImpl);
}

void UMPS::MessageFormats::Earthworm::swap(TraceBuf2<int16_t> &lhs,
                                           TraceBuf2<int16_t> &rhs)
{
    std::swap(lhs.pImpl, rhs.pImpl);
}


///--------------------------------------------------------------------------///
///                          Template Instantiation                          ///
///--------------------------------------------------------------------------///
template class UMPS::MessageFormats::Earthworm::TraceBuf2<double>;
template class UMPS::MessageFormats::Earthworm::TraceBuf2<float>;
template class UMPS::MessageFormats::Earthworm::TraceBuf2<int>;
template class UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>;

template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(const std::vector<double> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(const std::vector<float> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(const std::vector<int> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(const std::vector<int8_t> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(int nSamples, const double *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(int nSamples, const float *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(int nSamples, const int *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<double>::setData(int nSamples, const int16_t *x);

template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(const std::vector<double> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(const std::vector<float> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(const std::vector<int> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(const std::vector<int8_t> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(int nSamples, const double *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(int nSamples, const float *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(int nSamples, const int *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<float>::setData(int nSamples, const int16_t *x);

template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(const std::vector<double> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(const std::vector<float> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(const std::vector<int> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(const std::vector<int8_t> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(int nSamples, const double *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(int nSamples, const float *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(int nSamples, const int *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int>::setData(int nSamples, const int16_t *x);

template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(const std::vector<double> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(const std::vector<float> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(const std::vector<int> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(const std::vector<int8_t> &x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(int nSamples, const double *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(int nSamples, const float *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(int nSamples, const int *x);
template void UMPS::MessageFormats::Earthworm::TraceBuf2<int16_t>::setData(int nSamples, const int16_t *x);

