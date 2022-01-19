#ifndef UMPS_PROXYSERVICES_PACKETCACHE_SENSORRESPONSE_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_SENSORRESPONSE_HPP
#include <memory>
#include <unordered_set>
#include "umps/messageFormats/message.hpp"
#include "umps/proxyServices/packetCache/enums.hpp"
namespace UMPS::ProxyServices::PacketCache
{
/// @class SensorResponse "sensorResponse.hpp" "umps/proxyServices/packetCache/sensorResponse.hpp"
/// @brief This represents all available sensors in the cache.
/// @sa SensorRequest
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class SensorResponse : public MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    SensorResponse();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    SensorResponse(const SensorResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize
    ///                          this class.  On exit, responses' behavior is
    ///                          undefined.
    SensorResponse(SensorResponse &&response) noexcept;
    /// @}
    
    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] response  The response to copy to this.
    /// @result A deep copy of the input response.
    SensorResponse& operator=(const SensorResponse &response); 
    /// @brief Move assignment operator.
    /// @param[in,out] response  The response whose memory will be moved to
    ///                          this.  On exit, response's behavior is
    ///                          undefined.
    /// @result The memory from response moved to this.
    SensorResponse& operator=(SensorResponse &&response) noexcept;
    /// @}

    /// @name Sensors
    /// @{

    /// @result All the sensors currently in the capped collection.
    /// @note The sensor names are formatted as:
    ///       NETWORK.STATION.CHANNEL.LOCATION_CODE.
    [[nodiscard]] std::unordered_set<std::string> getNames() const noexcept;
    /// @brief Sets all the sensor names.
    /// @note Duplicate
    void setNames(const std::unordered_set<std::string> &names) noexcept;
    /// @} 

    /// @name Additional Information
    /// @{

    /// @brief Allows the service to set its return code and signal to
    ///        the requester whether or not the request was successfully
    ///        processed.
    /// @param[in] code  The return code.
    void setReturnCode(ReturnCode code) noexcept;
    /// @result The return code from the service.
    [[nodiscard]] ReturnCode getReturnCode() const noexcept;

    /// @brief For asynchronous messaging this allows the requester to index
    ///        the request.  This value will be returned so the requester
    ///        can track which request was filled by the response.
    /// @param[in] identifier   The request identifier.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result The request identifier.
    [[nodiscard]] uint64_t getIdentifier() const noexcept;
    /// @}

    /// @name Message Properties
    /// @{

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
    virtual void fromMessage(const char *data, size_t length) override final;
    /// @result A message type indicating this is a pick message.
    [[nodiscard]] virtual std::string getMessageType() const noexcept override final;
    /// @name Clone Functions
    /// @{
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept override final;
    /// @}

    /// @name Debugging Utilities
    /// @{

    /// @brief Creates the class from a JSON data reseponse message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the data response class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @brief Converts the pick class to a CBOR message.
    /// @result The class expressed in Compressed Binary Object Representation
    ///         (CBOR) format.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] std::string toCBOR() const;
    /// @brief Creates the class from a CBOR message.
    /// @param[in] cbor  The CBOR message.
    void fromCBOR(const std::string &cbor);
    /// @brief Creates the class from a CBOR message.
    /// @param[in] data    The contents of the CBOR message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NUL or length is 0.
    void fromCBOR(const uint8_t *data, size_t length);
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~SensorResponse() override;
    /// @}
private:
    class SensorResponseImpl;
    std::unique_ptr<SensorResponseImpl> pImpl;
};
}
#endif
