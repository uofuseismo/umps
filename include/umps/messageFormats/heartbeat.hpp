#ifndef UMPS_MESSAGEFORMATS_HEARTBEAT_HPP
#define UMPS_MESSAGEFORMATS_HEARTBEAT_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::MessageFormats
{
/// @brief Defines the module's status.
enum class HeartbeatStatus : int8_t
{
    UNKNOWN = 0,
    ALIVE,         /*!< The module is alive. */
    DISCONNECTED,  /*!< The module has safely disconnected. */
    DIED           /*!< The module has unexpectedly died.  This likely
                        indicates that a restart is required. */
};
/// @class Heartbeat "heartbeat.hpp" "umps/messageFormats/heartbeat.hpp"
/// @brief Defines a heartbeat message.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Heartbeat : public IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Heartbeat();
    /// @brief Copy constructor.
    /// @param[in] heartbeat  The hearbeat class which to initialize this class.
    Heartbeat(const Heartbeat &heartbeat);
    /// @brief Move constructor.
    /// @param[in,out] heartbeat  The heartbeat class from which to initialize
    ///                           this class.  On exit, heartbeat's behavior is
    ///                           undefined.
    Heartbeat(Heartbeat &&heartbeat) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] heartbeat  The heartbeat class to copy to this.
    /// @result A deep copy of the heartbeat.
    Heartbeat& operator=(const Heartbeat &heartbeat);
    /// @brief Move assignment.
    /// @param[in,out] heartbeat  The heartbeat class whose memory will be moved
    ///                           to this.  On exit heartbeat's behavior is
    ///                           undefined.
    /// @result The memory from heartbeat moved to this.
    Heartbeat& operator=(Heartbeat &&heartbeat) noexcept;
    /// @}

    /// @brief Sets the module's status.
    /// @param[in] status  The module's status.
    void setStatus(HeartbeatStatus status) noexcept;
    /// @result The module's status.  By default this is alive.
    [[nodiscard]] HeartbeatStatus getStatus() const noexcept;

    /// @brief Sets the host computer's name.  This helps identify the on
    ///        which computer the module is being run.  
    /// @param[in] name   The host name.
    void setHostName(const std::string &name);
    /// @result The hostname.
    [[nodiscard]] std::string getHostName() const noexcept;

    /// @brief Sets the time stamp.
    /// @param[in] timeStamp   Sets the time stamp.
    void setTimeStamp(const std::string &timeStamp);
    /// @brief Sets the time stamp to now.
    void setTimeStampToNow() noexcept;
    /// @result The time stamp.
    [[nodiscard]] std::string getTimeStamp() const noexcept;
 
    /// @name Message Abstract Base Class Properties
    /// @{
    /// @brief Converts the heartbeat class to a string message.
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
    virtual void fromMessage(const char *data, size_t length) override final;
    /// @result A message type indicating this is a heartbeat message.
    [[nodiscard]] virtual std::string getMessageType() const noexcept override final;
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept override final;
    /// @}

    /// @name Debugging Utilities
    /// @{
    /// @brief Creates the class from a JSON heartbeat message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the heartbeat class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @brief Converts the heartbeat class to a CBOR message.
    /// @result The class expressed in Compressed Binary Object Representation
    ///         (CBOR) format.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] virtual std::string toCBOR() const;
    /// @brief Creates the class from a CBOR message.
    /// @param[in] cbor  The CBOR message.
    void fromCBOR(const std::string &cbor);
    /// @brief Creates the class from a CBOR message.
    /// @param[in] data    The contents of the CBOR message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0.
    virtual void fromCBOR(const uint8_t *data, size_t length);
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~Heartbeat();
    /// @}
private:
    class HeartbeatImpl;
    std::unique_ptr<HeartbeatImpl> pImpl;
};
}
#endif
