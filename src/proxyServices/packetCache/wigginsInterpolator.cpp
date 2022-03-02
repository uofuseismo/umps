#include <iostream>
#include <chrono>
#include <cstdbool>
#include <algorithm>
#include "umps/proxyServices/packetCache/wigginsInterpolator.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "private/applications/wiggins.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UMF = UMPS::MessageFormats;

class WigginsInterpolator::WigginsInterpolatorImpl
{
public:
    std::vector<double> mSignal;
    std::vector<int8_t> mGapIndicator;
    double mTargetSamplingRate = 100;
    std::chrono::microseconds mGapTolerance{50000};
    std::chrono::microseconds mStartTime{0};
    std::chrono::microseconds mEndTime{0};
};

/// C'tor
WigginsInterpolator::WigginsInterpolator() :
    pImpl(std::make_unique<WigginsInterpolatorImpl> ())
{
}

/// Copy c'tor
WigginsInterpolator::WigginsInterpolator(const WigginsInterpolator &wiggins)
{
    *this = wiggins;
}

/// Move c'tor
WigginsInterpolator::WigginsInterpolator(WigginsInterpolator &&wiggins) noexcept
{
    *this = std::move(wiggins);
}

/// Copy assignment
WigginsInterpolator&
WigginsInterpolator::operator=(const WigginsInterpolator &wiggins)
{
    if (&wiggins == this){return *this;}
    pImpl = std::make_unique<WigginsInterpolatorImpl> (*wiggins.pImpl);
    return *this;
}

/// Move assignment
WigginsInterpolator&
WigginsInterpolator::operator=(WigginsInterpolator &&wiggins) noexcept
{
    if (&wiggins == this){return *this;}
    pImpl = std::move(wiggins.pImpl);
    return *this;
}

/// Destructor
WigginsInterpolator::~WigginsInterpolator() = default;

/// Target sampling rate
void WigginsInterpolator::setTargetSamplingRate(const double samplingRate)
{
    if (samplingRate <= 0)
    {
        throw std::invalid_argument("Sampling rate must be positive");
    }
    pImpl->mTargetSamplingRate = samplingRate;
}

double WigginsInterpolator::getTargetSamplingRate() const noexcept
{
    return pImpl->mTargetSamplingRate;
}

/// Gap tolerance
void WigginsInterpolator::setGapTolerance(
    const std::chrono::microseconds &tolerance) noexcept
{
    pImpl->mGapTolerance = tolerance;
} 

std::chrono::microseconds WigginsInterpolator::getGapTolerance() const noexcept
{
    return pImpl->mGapTolerance;
}

/// Number of samples in interpolated signal
int WigginsInterpolator::getNumberOfSamples() const noexcept
{
    return static_cast<int> (pImpl->mSignal.size());
}

/// Interpolate
template<typename T>
void WigginsInterpolator::interpolate(
    const std::vector<UMF::DataPacket<T>> &packets)
{
    pImpl->mSignal.clear();
    pImpl->mGapIndicator.clear();
    pImpl->mStartTime = std::chrono::microseconds {0};
    pImpl->mEndTime = std::chrono::microseconds {0};
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
    std::vector<std::pair<int64_t, int64_t>> startEndTimes;
    startEndTimes.reserve(nPackets);
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
            auto t1 = packets[iPacket].getEndTime().count();
            startEndTimes.push_back(std::pair(t0, t1)); 
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
    auto targetSamplingRate = pImpl->mTargetSamplingRate;
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
    //result.setSamplingRate(targetSamplingRate);
    //result.setStartTime(static_cast<double> (time0)/1000000);
    pImpl->mSignal = weightedAverageSlopes(times, data, timesToEvaluate,
                                           checkSorting);
    if (!pImpl->mSignal.empty())
    {
        pImpl->mStartTime = std::chrono::microseconds{time0};
        pImpl->mEndTime = std::chrono::microseconds{timesToEvaluate.back()}; 
        pImpl->mGapIndicator.resize(pImpl->mSignal.size(), 0);
    }
}

// Get signal
std::vector<double> WigginsInterpolator::getSignal() const noexcept
{
    return pImpl->mSignal;
}

const double *WigginsInterpolator::getSignalPointer() const noexcept
{
    return pImpl->mSignal.data();
}

// Get gap indicator
std::vector<int8_t> WigginsInterpolator::getGapIndicator() const noexcept
{
    return pImpl->mGapIndicator;
}

const int8_t *WigginsInterpolator::getGapIndicatorPointer() const noexcept
{
    return pImpl->mGapIndicator.data();
}

/// Start time
std::chrono::microseconds WigginsInterpolator::getStartTime() const noexcept
{
    return pImpl->mStartTime;
}

/// End time
std::chrono::microseconds WigginsInterpolator::getEndTime() const noexcept
{
    return pImpl->mEndTime;
}

/*
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
    std::vector<std::pair<int64_t, int64_t>> startEndTimes;
    startEndTimes.reserve(nPackets);
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
            auto t1 = packets[iPacket].getEndTime().count();
            startEndTimes.push_back(std::pair(t0, t1)); 
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
    // Determine which points were interpolated and which were extrapolated
    auto n = static_cast<int> (timesToEvaluate.size());
    std::vector<bool> wasInterpolated(timesToEvaluate.size(), false);
    for (const auto &startEndTime : startEndTimes) 
    {
        auto t0 = startEndTime.first;
        auto t1 = startEndTime.second;
        for (int i = 0; i < n; ++i)
        {
            if (timesToEvaluate[i] >= t0 && timesToEvaluate[i] <= t1)
            {
                wasInterpolated[i] = true;
            }
        }
    }
    return result;
}
*/

///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------///
template
void UMPS::ProxyServices::PacketCache::WigginsInterpolator::interpolate(
    const std::vector<UMF::DataPacket<double>> &packets);
template
void UMPS::ProxyServices::PacketCache::WigginsInterpolator::interpolate(
    const std::vector<UMF::DataPacket<float>> &packets);
/*
template UMF::DataPacket<double> UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<double>> &packets, double samplingRate);
template UMF::DataPacket<float> UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<float>> &packets, double samplingRate);
template UMF::DataPacket<int> UMPS::ProxyServices::PacketCache::interpolate(
    const std::vector<UMF::DataPacket<int>> &packets, double samplingRate);
*/
