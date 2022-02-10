#include <iostream>
#include <chrono>
#include <algorithm>
#include "umps/proxyServices/packetCache/interpolate.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "private/applications/wiggins.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UMF = UMPS::MessageFormats;

template<typename T>
void UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<T>> &packets,
    const double targetSamplingRate)
{
    // Does target sampling rate make sense?
    if (targetSamplingRate <= 0)
    {
        throw std::invalid_argument("Target sampling rate must be positive");
    }
    // Is there data?
    if (packets.empty()){throw std::invalid_argument("No data packets");}
    for (const auto &packet : packets)
    {
        if (!packet.haveSamplingRate())
        {
            throw std::invalid_argument(
               "Sampling rate must be set for all packets");
        }
    }
    // Is there data?
    auto nPackets = static_cast<int> (packets.size());
    std::vector<int> packetSamplePtr(nPackets + 1);
    int nSamples = 0;
    packetSamplePtr[0] = 0;
    for (int i = 0; i < nPackets; ++i)
    {
        nSamples = nSamples + packets[i].getNumberOfSamples();
        packetSamplePtr[i + 1] = nSamples;
    }
    if (nSamples < 1)
    {
        throw std::invalid_argument("No samples in packets");
    }
    if (nSamples < 2)
    {
        throw std::invalid_argument("At least two samples required");
    } 
    // Create data
    std::vector<T> data(nSamples);
    std::vector<int64_t> times(nSamples);
    for (int i = 0; i < nPackets; ++i)
    {
        const T *__restrict__ dataPtr = packets[i].getDataPointer();
        auto i0 = packetSamplePtr[i];
        auto i1 = packetSamplePtr[i + 1]; // Exclusive
        auto nSamplesInPacket = i1 - i0;
        if (nSamplesInPacket > 0)
        {
            std::copy(dataPtr, dataPtr + nSamplesInPacket, &data[i0]);
            auto samplingRate = packets[i].getSamplingRate();
            auto samplingRateMicroHertz
                = static_cast<int64_t> (std::round(samplingRate*1000000));
            auto t0 = packets[i].getStartTime().count();
            auto *__restrict__ timesPtr = &times[i0];
            for (int i = 0; i < nSamplesInPacket; ++i) 
            {
                timesPtr[i] = t0 + i*samplingRateMicroHertz;
            }
        }
    }
    // Interpolate
    int64_t time0 = times.front();
    int64_t time1 = times.back();
    auto iTargetSamplingRate = static_cast<int64_t> (std::round(targetSamplingRate*1000000));
    std::vector<int64_t> interpTimes;
    int i = 0;
    while (true)
    {
       auto timeInterp = time0 + i*iTargetSamplingRate; 
       if (timeInterp > time1){break;}
       interpTimes.push_back(timeInterp);
       i = i + 1;
    }
   
}

///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------////
template void UMPS::ProxyServices::PacketCache::interpolate(const std::vector<UMF::DataPacket<double>> &packets, double samplingRate);
template void UMPS::ProxyServices::PacketCache::interpolate(const std::vector<UMF::DataPacket<float>> &packets, double samplingRate);
template void UMPS::ProxyServices::PacketCache::interpolate(const std::vector<UMF::DataPacket<int>> &packets, double samplingRate);
