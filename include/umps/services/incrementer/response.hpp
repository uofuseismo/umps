#ifndef UMPS_SERVICES_INCREMENTER_RESPONSE_HPP
#define UMPS_SERVICES_INCREMENTER_RESPONSE_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
#include "umps/services/incrementer/enums.hpp"
namespace UMPS::Services::Incrementer
{
/// @class Response "response.hpp" "umps/services/incrementer/response.hpp"
/// @brief This is a response to an item increment request.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Response : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Response();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    Response(const Response &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize this
    ///                          class.  On exit, response's behavior is
    ///                          undefined.
    Response(Response &&response) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] response   The response to copy to this.
    /// @result A deep copy of the the input response.
    Response& operator=(const Response &response);
    /// @brief Move assignment operator.
    /// @param[in,out] resonse  The response whose memory will be moved to this.
    ///                         On exit, response's behavior is undefined.
    /// @result The memory from response moved to this.
    Response& operator=(Response &&response) noexcept;
    /// @}

    /// @name Response Information
    /// @{
    /// @brief Sets the increment value.
    /// @param[in] value  The increment value.
    void setValue(uint64_t value) noexcept; 
    /// @throws std::runtime_error if the increment value was not set.
    [[nodiscard]] uint64_t getValue() const;
    /// @result True indicates that the increment value was set.
    [[nodiscard]] bool haveValue() const noexcept;

    /// @brief Sets the request identifier.
    /// @param[in] identifier  The request identifier.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result The request identifier.
    [[nodiscard]] uint64_t getIdentifier() const noexcept;

    /// @brief Allows the incrementer to set its return code and signal to
    ///        the requester whether or not the increment was successful.
    /// @param[in] code   The return code.
    void setReturnCode(ReturnCode code) noexcept;
    /// @result The return code from the incrementer.
    [[nodiscard]] ReturnCode getReturnCode() const noexcept;
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
    virtual void fromMessage(const char *data, size_t length) override final;
    /// @result The message type - e.g., "DataPacket".
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @}

    /// @name (De)serialization Utilities
    /// @{
    /// @brief Creates the class from a JSON response message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the response class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @brief Convenience function to initialize this class from a CBOR
    ///        message.
    /// @param[in] cbor  The CBOR message held in a string container.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if the required information is not set.
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
    /// @} 
 
    /// @name Destructors
    /// @{
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~Response();
    /// @}
private:
    class ResponseImpl;
    std::unique_ptr<ResponseImpl> pImpl;
};
}
#endif
