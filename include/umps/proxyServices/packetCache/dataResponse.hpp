#ifndef UMPS_PROXYSERVICES_PACKETCACHE_DATARESPONSE_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_DATARESPONSE_HPP
#include <memory>
#include <vector>
#include "umps/messageFormats/message.hpp"
#include "umps/proxyServices/packetCache/enums.hpp"
namespace UMPS::MessageFormats
{
 template<class T> class DataPacket;
}
namespace UMPS::ProxyServices::PacketCache
{
/// @name DataResponse "dataResponse.hpp" "umps/proxyServices/packetCache/dataResponse.hpp"
/// @brief This represents the packet data for a sensor.
/// @sa DataRequest
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
class DataResponse : public MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    DataResponse();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    DataResponse(const DataResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize
    ///                          this class.  On exit, responses' behavior is
    ///                          undefined.
    DataResponse(DataResponse &&response) noexcept;
    /// @}
    
    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] response  The response to copy to this.
    /// @result A deep copy of the input response.
    DataResponse& operator=(const DataResponse &response); 
    /// @brief Move assignment operator.
    /// @param[in,out] response  The response whose memory will be moved to
    ///                          this.  On exit, response's behavior is
    ///                          undefined.
    /// @result The memory from response moved to this.
    DataResponse& operator=(DataResponse &&response) noexcept;
    /// @}

    /// @name Data Packets
    /// @{

    /// @brief Sets the data packets.
    /// @param[in] packets  The data packets corresponding to the request.
    /// @throws std::invalid_argument if any packet's network, station, channel,
    ///         location code, or sampling rate is not set.
    void setPackets(const std::vector<UMPS::MessageFormats::DataPacket<T>> &packets);
    /// @brief Sets the data packets.
    /// @param[in,out] packets  On input these are the data packets to set.
    ///                         On exit, packets's behavior is undefined.
    /// @throws std::invalid_argument if any packet's network, station, channel,
    ///         location code, or sampling rate is not set.
    void setPackets(std::vector<UMPS::MessageFormats::DataPacket<T>> &&packets);
    /// @result The number of packets.
    [[nodiscard]] int getNumberOfPackets() const noexcept;
    /// @result A pointer to the data packets.  This is an array with dimensions
    ///         [\c getNumberOfPackets()].
    [[nodiscard]] const UMPS::MessageFormats::DataPacket<T> *getPacketsPointer() const noexcept;
    /// @result The data packets corresponding to the data request. 
    /// @note If result.empty() then a problem was likely detected and you
    ///       should check the return code.
    [[nodiscard]] std::vector<UMPS::MessageFormats::DataPacket<T>> getPackets() const noexcept;
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
    [[nodiscard]] std::string toMessage() const final;
    /// @brief Creates the class from a message.
    /// @param[in] message  The message from which to create this class.
    /// @throws std::invalid_argument if message.empty() is true.
    /// @throws std::runtime_error if the message is invalid.
    void fromMessage(const std::string &message) final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, size_t length) final;
    /// @result A message type indicating this is a pick message.
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
    ~DataResponse() override;
    /// @}
private:
    class DataResponseImpl;
    std::unique_ptr<DataResponseImpl> pImpl;
};
}
#endif
