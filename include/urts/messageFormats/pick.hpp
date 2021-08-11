#ifndef URTS_MESSAGEFORMATS_PICK_HPP
#define URTS_MESSAGEFORMATS_PICK_HPP
#include <memory>
#include "urts/messageFormats/message.hpp"
namespace URTS::MessageFormats
{
/// @class Pick "pick.hpp" "urts/messageFormats/pick.hpp"
/// @brief Defines a pick message.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Pick : public IMessage
{
public:
    /// @brief The pick polarity.  For example, if this is a direct P arrival
    ///        arriving at a station with Z positive up then an "UP" polarity
    ///        indicate compression, "DOWN" would indicate dilitation, and
    ///        "UNKNOWN" would be indeterminate.
    enum class Polarity : int8_t
    {
        UNKNOWN = 0,/*!< Unknown polarity. */
        UP = 1,     /*!< The pick corresponds to positive polarity. */
        DOWN = -1   /*!< The pick corresponds to negative polarity. */
    };
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Pick();
    /// @brief Copy constructor.
    /// @param[in] pick  The pick class from which to initialize this class.
    Pick(const Pick &pick);
    /// @brief Move constructor.
    /// @param[in,out] pick  The pick class from which to initialize this
    ///                      class.  On exit, pick's behavior is undefined.
    Pick(Pick &&pick) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] pick  The pick class to copy to this.
    /// @result A deep copy of the pick.
    Pick& operator=(const Pick &pick);
    /// @brief Move assignment.
    /// @param[in,out] pick  The pick class whose memory will be moved to
    ///                      this.  On exit pick's behavior is undefined.
    /// @result The memory from pick moved to this.
    Pick& operator=(Pick &&pick) noexcept;
    /// @}

    /// @name Required Information
    /// @{
    /// @name Pick Time
    /// @{
    /// @brief Sets the pick time.
    /// @param[in] time  The time (UTC) of the arrival.  This is measured in
    ///                  seconds since the epoch (Jan 1 1970).
    void setTime(double time) noexcept;
    /// @result The time (UTC) of the pick in seconds since the epoch.
    /// @throws std::runtime_error if \c haveTime() is false.
    [[nodiscard]] double getTime() const;
    /// @result True indicates that the pick time was set.
    [[nodiscard]] bool haveTime() const noexcept;
    /// @}
 
    /// @name Site Identification
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
    /// @} 

    /// @name Identifier
    /// @brief Sets a unique pick identification number.
    /// @param[in] id   The unique pick identification number.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result The unique pick identification number.
    /// @throws std::runtime_error if \c haveIdentifier() is false.
    [[nodiscard]] uint64_t getIdentifier() const;
    /// @result True indicates that the pick identifier was set.
    [[nodiscard]] bool haveIdentifier() const noexcept;
    /// @} 

    /// @name Optional Information
    /// @{
    /// @brief Sets a guess of the arrival's seismic phase.
    void setPhaseHint(const std::string &phase) noexcept;
    /// @result The phase hint.
    /// @note If this was not set then result.empty() will be true.
    [[nodiscard]] std::string getPhaseHint() const noexcept;

    /// @brief Sets the polarity of the arrival.
    /// @param[in] polarity   The pick's polarity.
    void setPolarity(Polarity polarity) noexcept;
    /// @result The pick's polarity.
    [[nodiscard]] Polarity getPolarity() const noexcept;

    /// @brief Sets the algorithm
    void setAlgorithm(const std::string &algorithm) noexcept;
    /// @result The algorithm that created this picked.
    [[nodiscard]] std::string getAlgorithm() const noexcept;
    /// @}

    /// @brief Converts the pick class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result The pick class expressed as a JSON message.
    /// @throws std::runtime_error if the required information is not set.
    [[nodiscard]] std::string toJSON(int noIndent =-1) const;
    /// @brief Creates the class from a JSON pick message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the pick class to a CBOR message.
    /// @result The class expressed in Compressed Binary Object Representation
    ///         (CBOR) format.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] virtual std::string toCBOR() const override final;
    /// @brief Creates the class from a CBOR message.
    /// @param[in] cbor  The CBOR message.
    void fromCBOR(const std::string &cbor);
    /// @brief Creates the class from a CBOR message.
    /// @param[in] data    The contents of the CBOR message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NUL or length is 0.
    virtual void fromCBOR(const uint8_t *data, size_t length) override final;
    /// @result A message type indicating this is a pick message.
    [[nodiscard]] virtual std::string getMessageType() const noexcept override final;

    /// @name Clone Functions
    /// @{
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<URTS::MessageFormats::IMessage> clone() const override final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] virtual std::unique_ptr<URTS::MessageFormats::IMessage> createInstance() const noexcept override final;
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~Pick();
    /// @}
private:
    class PickImpl;
    std::unique_ptr<PickImpl> pImpl;
};
}
#endif
