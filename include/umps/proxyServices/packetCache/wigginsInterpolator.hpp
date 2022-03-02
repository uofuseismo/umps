#ifndef UMPS_PROXYSERVICES_PACKETCACHE_WIGGINSINTERPOLATOR_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_WIGGINSINTERPOLATOR_HPP
#include <memory>
#include <vector>
namespace UMPS
{
 namespace MessageFormats
 {
  template<class T> class DataPacket;
 }
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class WigginsInterpolator "wigginsInterpolator.hpp" "umps/proxyServices/packetCache/wigginsInterpolator.hpp"
/// @brief Performs Wiggins interpolation on a vector of data packets. 
///        This can heal the packets returned by a packet cache query.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class WigginsInterpolator
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    WigginsInterpolator();
    /// @brief Copy constructor.
    /// @param[in] wiggins  The interpolator from which to initialize this
    ///                     class. 
    WigginsInterpolator(const WigginsInterpolator &wiggins);
    /// @brief Move constructor.
    /// @param[in,out] wiggins  The interpolator from which to initialize this
    ///                         class.  On exit, wiggins's behavior is
    ///                         undefined.
    WigginsInterpolator(WigginsInterpolator &&interpolate) noexcept;
    /// @}

    /// @name Operators
    /// @{

    WigginsInterpolator& operator=(const WigginsInterpolator &wiggins);
    WigginsInterpolator& operator=(WigginsInterpolator &&wiggins) noexcept;
    /// @}

    /// @name Target Sampling Rate
    /// @{

    /// @brief Sets the target sampling rate.  This is the sampling rate to
    ///        which signals will be interpolated.
    /// @param[in] samplingRate  The sampling rate, in Hz, to which the packets
    ///                          will be interpolated. 
    /// @throws std::invalid_argument if this is not positive.
    void setTargetSamplingRate(const double samplingRate);
    /// @result The nominal sampling ratein Hz.  By default this is 100 Hz.
    [[nodiscard]] double getTargetSamplingRate() const noexcept;
    /// @}

    /// @name Gap Tolerance
    /// @{

    /// @brief Sets the gap tolerance between packets.
    /// @param[in] tolerance  If the time between the end of a packet and the
    ///                       start of the next packet exceeds this time
    ///                       then the samples interpolated between the packets
    ///                       will be denoted as extrapolated data gaps.
    /// @note Zero or negative effectively disables this.  In which case any
    ///       sample interpolated between a packet is being extrapolated in a 
    ///       gap.
    void setGapTolerance(const std::chrono::microseconds &tolerance) noexcept;
    /// @result The gap tolerance.  By default this is 30000 (0.03 seconds)
    ///         or 3 samples at the nominal sampling rate of 100 Hz.
    std::chrono::microseconds getGapTolerance() const noexcept;
    /// @}

    /// @name Interpolate
    /// @{
    /// @brief Interpolates the data packets.  This will interpolate from the
    ///        earliest time in the colleciton of packets up to the latest
    ///        time in the packets.
    /// @param[in] packets  The packets to interpolate.
    template<typename T>
    void interpolate(const std::vector<MessageFormats::DataPacket<T>> &packets);
    /// @}

    /// @name Interpolated Signal
    /// @{
   
    /// @result The interpolated signal.
    [[nodiscard]] std::vector<double> getSignal() const noexcept;
    /// @result A pointer to the interpolated signal.  This is an array whose
    ///         dimension is [\c getNumberOfSamples].
    /// @note This exists for performance reasons.  When possible use
    ///       \c getSignal().
    [[nodiscard]] const double *getSignalPointer() const noexcept;
    /// @result The number of samples in the interpolated signal.
    [[nodiscard]] int getNumberOfSamples() const noexcept;

    /// @result An array of true/false that indicates whether or not the 
    ///         interpolated signal fell between packet endpoints.   
    [[nodiscard]] std::vector<int8_t> getGapIndicator() const noexcept;
    /// @result A pointer to a signal that indicates whether or not the
    ///         sample in the interpolated signal was within a packet (false) or
    ///         between packets (true) - i.e., extrapolated.
    [[nodiscard]] const int8_t *getGapIndicatorPointer() const noexcept;

    /// @result The start time of the interpolated signal.
    [[nodiscard]] std::chrono::microseconds getStartTime() const noexcept;
    /// @result The end time of the interpolated signal.
    [[nodiscard]] std::chrono::microseconds getEndTime() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Resets the start/end time to zero and releases memory of the
    ///        gap pointer and signal.  This will modify the gap tolerance or
    ///        target sampling rate.
    void clearSignal() noexcept; 
    /// @brief Destructor.
    ~WigginsInterpolator();
    /// @}
private:
    class WigginsInterpolatorImpl;
    std::unique_ptr<WigginsInterpolatorImpl> pImpl;
};
template<typename T>
struct InterpolatedSignal
{
public:
    std::vector<T> mSignal;
    std::vector<bool> mWasInterpolated;
    std::chrono::microseconds mStartTime{0}; 
    double mSamplingRate = 0;
};
/// @brief Interpolates a collection of data packets.  This will interpolate 
///        from the earliest time in the collection of packets up to the
///        the last time.
/// @param[in] packets   The packets to interpolate.
/// @param[in] targetSamplingRate  The sampling rate of the output signal in Hz.
/// @result A container with the interpolated signal.
/// @note The interpolation is actually performed in double precision.  For
///       that reason there can be a significant loss of precision if
///       using the integer template variant.
template<typename T>
[[deprecated]]
[[nodiscard]]
MessageFormats::DataPacket<T>
interpolate(const std::vector<MessageFormats::DataPacket<T>> &packets,
            double targetSamplingRate = 100);
}
#endif
