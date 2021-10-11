#ifndef UMPS_MESSAGEFORMATS_EARTHWORM_TRACEBUF2
#define UMPS_MESSAGEFORMATS_EARTHWORM_TRACEBUF2
#include <memory>
#include <vector>
#include <string>
#include "umps/messageFormats/message.hpp"
namespace UMPS::MessageFormats::Earthworm
{
/// @name TraceBuf2 "tracebuf2.hpp" "umps/messageFormats/earthworm/tracebuf2.hpp"
/// @brief Defines an Earthworm tracebuf2 message format.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
class TraceBuf2 : public IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor
    TraceBuf2();
    /// @brief Copy constructor.
    /// @param[in] traceBuf2  The tracebuf2 from which to initialize this class.
    TraceBuf2(const TraceBuf2 &traceBuf2);
    /// @brief Move constructor.
    /// @param[in] traceBuf2  The tracebuf2 from which to initialize this class.
    ///                       On exit, tracebuf2's behavior is undefined.
    TraceBuf2(TraceBuf2 &&traceBuf2) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] traceBuf2  The traceBuf2 class to copy to this.
    /// @result A deep copy of the input traceBuf2.
    TraceBuf2& operator=(const TraceBuf2 &traceBuf2);
    /// @brief Move assignment.
    /// @param[in,out] traceBuf2  The traceBuf2 class whose memory will be
    ///                           moved to this.  On exit, traceBuf2's 
    ///                           behavior is undefined.
    /// @result The memory from traceBuf2 moved to this.
    TraceBuf2& operator=(TraceBuf2 &&traceBuf2) noexcept;
    /// @}

    /// @name Trace Header Information
    /// @{
    /// @param[in] pinNumber  The pin number.
    void setPinNumber(int pinNumber) noexcept;
    /// @result The pin number.
    [[nodiscard]] int getPinNumber() const noexcept;
    
    /// @brief Sets the start time.
    /// @param[in] startTime   The UTC time of the first sample in seconds from
    ///                        the epoch (January 1 1970). 
    void setStartTime(double startTime) noexcept;
    /// @result The UTC time of the first sample in seconds from the epoch.
    [[nodiscard]] double getStartTime() const noexcept;

    /// @result The UTC time of the last sample in seconds from the epoch.
    /// @throws std::runtime_error if \c haveSamplingRate() is false
    ///         \c getNumberOfSamples() is zero.
    [[nodiscard]] double getEndTime() const;

    /// @brief Sets the sampling rate for the data in the packet.
    /// @param[in] samplingRate   The sampling rate in Hz.
    /// @throws std::invalid_argument if the sampling rate is not positive.
    void setSamplingRate(double samplingRate);
    /// @result The sampling rate in Hz.
    /// @throws std::runtime_error if \c haveSamplingRate() is false.
    [[nodiscard]] double getSamplingRate() const;
    /// @result True indicates that the sampling rate was set.
    [[nodiscard]] bool haveSamplingRate() const noexcept;
    
    /// @result The number of samples.
    /// @note The number of samples is set when setting the data.
    [[nodiscard]] int getNumberOfSamples() const noexcept;

    /// @result The maximum number of samples that can be packed into a message
    ///         and put onto an earthworm ring.
    [[nodiscard]] int getMaximumNumberOfSamples() const noexcept;

    /// @brief Sets the network code.
    /// @param[in] network   The network name.
    /// @note If this is larger that \c getMaximumNetworkLength()
    ///       then it will be truncated.
    void setNetwork(const std::string &network) noexcept;
    /// @result The network name. 
    [[nodiscard]] std::string getNetwork() const noexcept;
    /// @result The maximum network code length.  This is likely 8.
    [[nodiscard]] int getMaximumNetworkLength() const noexcept;

    /// @brief Sets the station name.
    /// @param[in] station  The station name.
    /// @note If this is larger that \c getMaximumStationLength()
    ///       then it will be truncated truncated.
    void setStation(const std::string &station) noexcept;
    /// @result The station name. 
    [[nodiscard]] std::string getStation() const noexcept;
    /// @result The maximum station name length.  This is likely 6.
    [[nodiscard]] int getMaximumStationLength() const noexcept;

    /// @brief Sets the channel name.
    /// @param[in] channel  The channel name.
    /// @note If this is larger that \c getMaximumChannelength()
    ///       then it will be truncated truncated.
    void setChannel(const std::string &channel) noexcept;
    /// @result The channel name. 
    [[nodiscard]] std::string getChannel() const noexcept;
    /// @result The maximum channel name length.  This is likely 3.
    [[nodiscard]] int getMaximumChannelLength() const noexcept;

    /// @brief Sets the location code.
    /// @param[in] location   The location code.
    /// @note If this is larger that \c getMaximumLocationCodeLength()
    ///       then it will be truncated truncated.
    void setLocationCode(const std::string &location) noexcept;
    /// @result The location code.
    [[nodiscard]] std::string getLocationCode() const noexcept;
    /// @result The maximum location code length.  This is likely 2.
    [[nodiscard]] int getMaximumLocationCodeLength() const noexcept;

    /// @result The data type.  This is for earthworm compability.  The 
    ///         underlying template type dictates.
    [[nodiscard]] std::string getDataType() const noexcept;

    /// @brief A data quality indicator.
    /// @param[in] quality  The quality flag.
    /// @note From SEED we have:
    ///       Amplifier saturation detected = 1,
    ///       Digitizer clipping detected = 2,
    ///       Spikes detected = 3,
    ///       Glitches detected = 4,
    ///       Missing/padded data present = 16,
    ///       Telemetry synchronization error = 32,
    ///       A digital filter may be charging = 64,
    ///       Time tag is questionable = 128
    void setQuality(int quality) noexcept;      
    /// @result The quality.
    [[nodiscard]] int getQuality() const noexcept;

    /// @result The version.
    [[nodiscard]] std::string getVersion() const noexcept;
    /// @}


    /// @name Data
    /// @{
    /// @param[in,out] data  The trace data to set.  On exit, data's behavior is
    ///                      undefined.
    void setData(std::vector<T> &&data) noexcept;
    /// @param[in] data  The trace data to set.
    template<typename U> void setData(const std::vector<U> &data);
    /// @param[in] nSamples  The number of samples.
    /// @param[in] data      The data to set on the class.  This is an array
    ///                      whose dimension is [nSamples].
    template<typename U> void setData(const int nSamples, const U *data);

    /// @result The data in the packet.
    [[nodiscard]] std::vector<T> getData() const noexcept;
    /// @result A pointer to the data in the packet.  This is an array whose
    ///         dimension is [\c getNumberOfSamples() ].
    /// @throws std::runtime_error if \c haveData() is false.
    /// @note This function exists for performance reasons and it is not
    ///       recommended for general use.
    [[nodiscard]] const T* getDataPointer() const;
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~TraceBuf2();
    /// @}

    /// @name (De)serialization
    /// @{
    /// @brief Unpacks a tracebuf2 message from the earthworm ring.
    /// @param[in] message   The earthworm message.
    /// @throws std::runtime_error if the message is invalid or NULL.
    void fromEarthworm(const char *message);
    /// @brief Converts this class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result This class expressed as a JSON message.
    [[nodiscard]] std::string toJSON(int nIndent = -1) const;
    /// @brief Creates the class from a JSON tracebuf2 message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);

    /// @result The CBOR message corresponding to this class.
    [[nodiscard]] std::string toCBOR() const;
    /// @brief Creates this class from a CBOR message.
    void fromCBOR(const std::string &cbor);
    /// @brief Creates this class from a CBOR message.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or the length is 0.
    void fromCBOR(const uint8_t *data, const size_t length);
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An instance of an uninitialized class.
    virtual std::unique_ptr<IMessage> createInstance() const noexcept override final;
    /// @brief Converts the packet class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] virtual std::string toMessage() const override final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, const size_t length) override final;
    /// @result A string descriptor for this message type.
    [[nodiscard]] 
    virtual std::string getMessageType() const noexcept override final;
    /// @}

    /// @brief Swaps two tracebuf2 classes.
    /// @param[in,out] lhs  Class to exchange with rhs.
    /// @param[in,out] rhs  Class to exchange with lhs.
    friend void swap(TraceBuf2<double>  &lhs, TraceBuf2<double>  &rhs);
    friend void swap(TraceBuf2<float>   &lhs, TraceBuf2<float>   &rhs);
    friend void swap(TraceBuf2<int>     &lhs, TraceBuf2<int>     &rhs);
    friend void swap(TraceBuf2<int16_t> &lhs, TraceBuf2<int16_t> &rhs);
private:
    class TraceBuf2Impl;
    std::unique_ptr<TraceBuf2Impl> pImpl;
};
/// @brief Swaps two traceBuf2 classes, lhs and rhs.
/// @param[in,out] lhs  On exit this will contain the information in rhs.
/// @param[in,out] rhs  On exit this will contain the information in lhs.
void swap(TraceBuf2<double> &lhs, TraceBuf2<double> &rhs);
void swap(TraceBuf2<float> &lhs, TraceBuf2<float> &rhs);
void swap(TraceBuf2<int> &lhs, TraceBuf2<int> &rhs);
void swap(TraceBuf2<int16_t> &lhs, TraceBuf2<int16_t> &rhs);

/// @brief Converts a traceBuf2 to a JSON message. 
/// @param[in] traceBuf  The tracebuf2 structure to convert to JSON.
/// @result The corresponding JSON message.
template<typename T>
std::string toJSON(const TraceBuf2<T> &traceBuf);
/// @brief Converts a JSON traceBuf2 message to a tracBuf2 class.
/// @param[in] message  The tracebuf2 message.
/// @result The corresponding TraceBuf2 class.
template<typename T>
TraceBuf2<T> fromJSON(const std::string &message);
}
#endif
