#ifndef UMPS_MESSAGEFORMATS_DATAPACKET_HPP
#define UMPS_MESSAGEFORMATS_DATAPACKET_HPP
#include <vector>
#include <chrono>
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::MessageFormats
{
/// @class DataPacket "dataPacket.hpp" "umps/messageFormats/dataPacket.hpp"
/// @brief Defines a packet of seismic data (a data packet).
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Messages_MessageFormats
template<class T>
class DataPacket : public IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    DataPacket();
    /// @brief Copy constructor.
    /// @param[in] packet  The data packet from which to initialize this class.
    DataPacket(const DataPacket &packet);
    /// @brief Move constructor.
    /// @param[in,out] packet  The data packet from which to initialize this
    ///                        class.  On exit, packet's behavior is undefined.
    DataPacket(DataPacket &&packet) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] packet  The packet to copy to this class.
    /// @result A deep copy of the input packet.
    DataPacket& operator=(const DataPacket &packet);
    /// @brief Move assignment.
    /// @param[in,out] packet  The packet whose memory will be moved to
    ///                        this class.
    /// @result The memory from packet moved to this.
    DataPacket& operator=(DataPacket &&packet) noexcept; 
    /// @}
 
    /// @name Required Information
    /// @{

    /// @brief Sets the network code on which the pick was made.
    /// @param[in] network  The network code.
    /// @throws std::invalid_argument if network is empty.
    void setNetwork(const std::string &network);
    /// @result The network code.
    /// @throws std::runtime_error if \c haveNetwork() is false.
    [[nodiscard]] std::string getNetwork() const;
    /// @result True indicates that the network was set.
    [[nodiscard]] bool haveNetwork() const noexcept;

    /// @brief Sets the station name on which the pick was made.
    /// @param[in] station   The station name.
    /// @throws std::invalid_argument if station is empty.
    void setStation(const std::string &station);
    /// @result The station name.
    /// @throws std::runtime_error if \c haveStation() is false.
    [[nodiscard]] std::string getStation() const;
    /// @result True indicates that the station name was set.
    [[nodiscard]] bool haveStation() const noexcept;

    /// @brief Sets the channel name on which the pick was made.
    /// @param[in] channel  The channel name.
    /// @throws std::invalid_argument if channel is empty.
    void setChannel(const std::string &channel);
    /// @result The channel name.
    /// @throws std::runtime_error if the channel was not set.
    [[nodiscard]] std::string getChannel() const;
    /// @result True indicates that the channel was set.
    [[nodiscard]] bool haveChannel() const noexcept;

    /// @brief Sets the location code.
    /// @param[in] location  The location code.
    /// @throws std::invalid_argument if location is empty.
    void setLocationCode(const std::string &location);
    /// @brief Sets the channel code on which the pick was made.
    /// @throws std::runtime_error if \c haveLocationCode() is false.
    [[nodiscard]] std::string getLocationCode() const;
    /// @result True indicates that the location code was set.
    [[nodiscard]] bool haveLocationCode() const noexcept;

    /// @brief Sets the sampling rate for data in the packet.
    /// @param[in] samplingRate  The sampling rate in Hz.
    /// @throws std::invalid_argument if samplingRate is not positive.
    void setSamplingRate(const double samplingRate);
    /// @result The sampling rate of the packet in Hz.
    /// @throws std::runtime_error if \c haveSamplingRate() is false.
    [[nodiscard]] double getSamplingRate() const;
    /// @result True indicates that the sampling rate was set.
    [[nodiscard]] bool haveSamplingRate() const noexcept; 
    /// @}

    /// @name Optional Information
    /// @{

    /// @param[in] startTime  The UTC start time in seconds from the epoch
    ///                       (Jan 1, 1970).
    void setStartTime(double startTime) noexcept;
    /// @param[in] startTime  The UTC start time in microseconds
    ///                       from the epoch (Jan 1, 1970). 
    void setStartTime(const std::chrono::microseconds &startTime) noexcept;
    /// @result The UTC start time in microseconds from the epoch.
    [[nodiscard]] std::chrono::microseconds getStartTime() const noexcept;
    //[[nodiscard]] int64_t getStartTime() const noexcept;
    /// @result The UTC time in microseconds from the epoch of the last sample.
    /// @throws std::runtime_error if \c haveSamplingRate() is false or
    ///         \c getNumberOfSamples() is 0.
    [[nodiscard]] std::chrono::microseconds getEndTime() const;
    /// @}

    /// @name Data
    /// @{

    /// @brief Sets the time series data in this packet.
    /// @param[in] data  The time series data.
    template<typename U>
        void setData(const std::vector<U> &data) noexcept;
    /// @brief Sets the time series data in this packet.
    /// @param[in,out] data  The time series data whose memory will be moved
    ///                      into the class.  On exit, data's behavior is
    ///                      undefined.
    void setData(std::vector<T> &&data) noexcept;
    /// @brief Sets the time series data in this packet.
    /// @param[in] nSamples  The nubmer of samples in the signal.
    /// @param[in] data      The time series data.  This is an array whose
    ///                      dimension is [nSamples].
    /// @throws std::invalid_argument if data is null.
    ///  
    template<typename U>
        void setData(const int nSamples, const U *data);
    /// @result The time series currently set on the packet. 
    [[nodiscard]] std::vector<T> getData() const noexcept;
    /// @result A pointer to the underlying data packet.  This is an array whose
    ///         dimensions is [\c getNumberOfSamples()] 
    [[nodiscard]] const T *getDataPointer() const noexcept;
    /// @result The number of data samples in the packet.
    [[nodiscard]] int getNumberOfSamples() const noexcept;
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
    /// @result The message type - e.g., "DataPacket".
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @}

    /// @name Debugging Utilities
    /// @{

    /// @brief Convenience function to initialize this class from a CBOR
    ///        message.
    /// @param[in] cbor  The CBOR message held in a string container.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromCBOR(const std::string &cbor);
    /// @brief Creates the class from a CBOR message.
    /// @param[in] data    The contents of the CBOR message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromCBOR(const uint8_t *data, const size_t length);
    /// @brief Converts the packet class to a CBOR message.
    /// @result The class expressed in Compressed Binary Object Representation
    ///         (CBOR) format.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] std::string toCBOR() const;
    /// @brief Creates the class from a JSON data packet message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the data packet class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~DataPacket() override;
    /// @}
private:
    class DataPacketImpl;
    std::unique_ptr<DataPacketImpl> pImpl;
};
}
#endif
