#ifndef UMPS_PROXYBROADCASTS_HEARTBEAT_STATUS_HPP
#define UMPS_PROXYBROADCASTS_HEARTBEAT_STATUS_HPP
#include <memory>
#include <ostream>
#include "umps/messageFormats/message.hpp"
namespace UMPS::ProxyBroadcasts::Heartbeat
{
/// @brief Defines the module's status.
enum class ModuleStatus : int8_t
{
    Unknown = 0,   /*!< The module's status is unknown. */
    Alive,         /*!< The module is alive. */
    Disconnected,  /*!< The module has safely disconnected. */
    Died           /*!< The module has unexpectedly died.  This likely
                        indicates that a restart is required. */
};
/// @class Status "status.hpp" "umps/broadcasts/heartbeat/status.hpp"
/// @brief Defines a modules status message.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Status : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Status();
    /// @brief Copy constructor.
    /// @param[in] status  The hearbeat status class which to initialize
    ///                    this class.
    Status(const Status &status);
    /// @brief Move constructor.
    /// @param[in,out] status  The heartbeat status class from which to 
    ///                        initialize this class.  On exit, status's
    ///                        behavior is undefined.
    Status(Status &&status) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] status  The heartbeat status class to copy to this.
    /// @result A deep copy of the status.
    Status& operator=(const Status &status);
    /// @brief Move assignment.
    /// @param[in,out] status  The heartbeat status class whose memory will
    ///                        be moved to this.  On exit status's behavior is
    ///                        undefined.
    /// @result The memory from status moved to this.
    Status& operator=(Status &&status) noexcept;
    /// @}


    /// @brief Sets the module name that is emitting heartbeat's.  This can
    ///        also be a service or a broadcast.
    /// @param[in] name   The name of the module.
    /// @throws std::invalid_argument if name is empty.
    void setModule(const std::string &name);
    /// @result The module's status.
    [[nodiscard]] std::string getModule() const noexcept;
 
    /// @brief Sets the module's status.
    /// @param[in] status  The module's status.
    void setModuleStatus(ModuleStatus status) noexcept;
    /// @result The module's status.  By default this is alive.
    [[nodiscard]] ModuleStatus getModuleStatus() const noexcept;

    /// @brief Sets the host computer's name.  This helps identify the on
    ///        which computer the module is being run.  
    /// @param[in] name   The host name.
    void setHostName(const std::string &name);
    /// @result The hostname.
    [[nodiscard]] std::string getHostName() const noexcept;

    /// @brief Sets the time stamp.
    /// @param[in] timeStamp   Sets the time stamp.  This must be a length
    ///                        23 string with the format:
    ///                        XXXX:XX:XXTXX:XX:XX.XXX
    ///                        which is ordered:
    ///                        Year, Month, Day of Month,
    ///                        Hour, Minute, Second, Millisecond
    /// @throws std::invalid_argument if the time stamp is incorrectly
    ///         formatted.
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
    [[nodiscard]] std::string toMessage() const final;
    /// @brief Creates the class from a message.
    /// @param[in] message   The status message.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argmument if message.empty() is true.
    void fromMessage(const std::string &message) final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, size_t length) final;
    /// @result A message type indicating this is a heartbeat status message.
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @result The message version.
    [[nodiscard]] std::string getMessageVersion() const noexcept final;
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept final;
    /// @}

    /// @name Debugging Utilities
    /// @{

    /// @brief Creates the class from a JSON status message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the status class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @brief Converts the status class to a CBOR message.
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
    void fromCBOR(const uint8_t *data, size_t length);
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~Status() override;
    /// @}
private:
    class StatusImpl;
    std::unique_ptr<StatusImpl> pImpl;
};
/// @result True indicates that the lhs status has a larger time stamp
///         than the right hand side status's time stamp.
[[nodiscard]] bool operator>(const Status &lhs, const Status &rhs);
/// @result A string representation of the status.
std::ostream& operator<<(std::ostream &os, const Status &status);
}
#endif
