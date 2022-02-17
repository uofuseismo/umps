#include <iostream>
#include <chrono>
#include <algorithm>
#include "umps/proxyServices/packetCache/interpolate.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "private/applications/wiggins.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UMF = UMPS::MessageFormats;

template<typename T>
UMF::DataPacket<T> UMPS::ProxyServices::PacketCache::interpolate(
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
    // Do all packets have sampling rates
    for (const auto &packet : packets)
    {
        if (!packet.haveSamplingRate())
        {
            throw std::invalid_argument(
               "Sampling rate must be set for all packets");
        }
    }
    UMF::DataPacket<T> result;
    // Is there data?
    auto nPackets = static_cast<int> (packets.size());
    std::vector<int> packetSamplePtr(nPackets + 1);
    int nSamples = 0;
    packetSamplePtr[0] = 0;
    for (int iPacket = 0; iPacket < nPackets; ++iPacket)
    {
        nSamples = nSamples + packets[iPacket].getNumberOfSamples();
        packetSamplePtr[iPacket + 1] = nSamples;
    }
    if (nSamples < 1)
    {
        throw std::invalid_argument("No samples in packets");
    }
    if (nSamples < 2)
    {
        throw std::invalid_argument("At least two samples required");
    } 
    // Are the packets in order?
    auto isSorted
        = std::is_sorted(packets.begin(), packets.end(), 
                         [](const UMF::DataPacket<T> &lhs, 
                            const UMF::DataPacket<T> &rhs)
                         {
                            return lhs.getEndTime() < rhs.getStartTime();
                         });
    // Create abscissas and values at abscissas
    std::vector<T> data(nSamples);
    std::vector<int64_t> times(nSamples);
    for (int iPacket = 0; iPacket < nPackets; ++iPacket)
    {
        const T *__restrict__ dataPtr = packets[iPacket].getDataPointer();
        auto i0 = packetSamplePtr[iPacket];
        auto i1 = packetSamplePtr[iPacket + 1]; // Exclusive
        auto nSamplesInPacket = i1 - i0;
        if (nSamplesInPacket > 0)
        {
            std::copy(dataPtr, dataPtr + nSamplesInPacket, &data[i0]);
            auto samplingRate = packets[iPacket].getSamplingRate();
            auto samplingPeriodMicroSeconds
                = static_cast<double> (1000000./samplingRate);
            auto t0 = packets[iPacket].getStartTime().count();
            auto *__restrict__ timesPtr = &times[i0];
            for (int i = 0; i < nSamplesInPacket; ++i)
            {
                timesPtr[i] = t0 + i*samplingPeriodMicroSeconds;
            }
        }
    }
    // Create the interpolation times
    int64_t time0, time1;
    if (isSorted)
    {
        time0 = times.front();
        time1 = times.back();
    }
    else
    {
        auto tMinMax = std::minmax_element(times.begin(), times.end());
        time0 = *tMinMax.first;
        time1 = *tMinMax.second;
    }
    auto targetSamplingPeriodMicroSeconds
        = static_cast<int64_t> (std::round(1000000./targetSamplingRate));
    auto spaceEstimate
        = static_cast<int>
          (std::round((time1 - time0)
                     /static_cast<double> (targetSamplingPeriodMicroSeconds)));
    std::vector<int64_t> timesToEvaluate;
    timesToEvaluate.reserve(std::max(1, spaceEstimate));
    int i = 0;
    while (true)
    {
       auto interpolationTime = time0 + i*targetSamplingPeriodMicroSeconds; 
       if (interpolationTime > time1){break;}
       timesToEvaluate.push_back(interpolationTime);
       i = i + 1;
    }
    // Package into the result
    auto checkSorting = !isSorted;
    result.setSamplingRate(targetSamplingRate);
    result.setStartTime(static_cast<double> (time0)/1000000);
    auto interpolatedSignal =
        weightedAverageSlopes(times, data, timesToEvaluate, checkSorting);
    if (!interpolatedSignal.empty())
    {
        result.setData(std::move(interpolatedSignal));
    }
    return result;
}

///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------///
template UMF::DataPacket<double> UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<double>> &packets, double samplingRate);
template UMF::DataPacket<float> UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<float>> &packets, double samplingRate);
template UMF::DataPacket<int> UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<int>> &packets, double samplingRate);
