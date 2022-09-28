#ifndef UMPS_PROXYSERVICES_INCREMENTER_ITEMSRESPONSE_HPP
#define UMPS_PROXYSERVICES_INCREMENTER_ITEMSRESPONSE_HPP
#include <memory>
#include <set>
#include "umps/messageFormats/message.hpp"
#include "umps/proxyServices/incrementer/enums.hpp"
namespace UMPS::ProxyServices::Incrementer
{
/// @class ItemsResponse "itemsResponse.hpp" "umps/proxyServices/incrementer/itemsResponse.hpp"
/// @brief This is a response to an items request.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class ItemsResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ItemsResponse();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    ItemsResponse(const ItemsResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize this
    ///                          class.  On exit, response's behavior is
    ///                          undefined.
    ItemsResponse(ItemsResponse &&response) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] response   The response to copy to this.
    /// @result A deep copy of the the input response.
    ItemsResponse& operator=(const ItemsResponse &response);
    /// @brief Move assignment operator.
    /// @param[in,out] response  The response whose memory will be moved to this.
    ///                          On exit, response's behavior is undefined.
    /// @result The memory from response moved to this.
    ItemsResponse& operator=(ItemsResponse &&response) noexcept;
    /// @}

    /// @name Response Information
    /// @{

    /// @brief Sets the increment value.
    /// @param[in] items  The unique item names to set.
    void setItems(const std::set<std::string> &items);
    /// @result The items being incremented.
    /// @throws std::runtime_error if the \c haveItems() is false.
    [[nodiscard]] std::set<std::string> getItems() const;
    /// @result True indicates that the increment value was set.
    [[nodiscard]] bool haveItems() const noexcept;

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
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final;
    /// @result An instance of an uninitialized class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept final;
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
    /// @result The message type - e.g., "DataPacket".
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @result The message version.
    [[nodiscard]] std::string getMessageVersion() const noexcept final;
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
    void fromCBOR(const uint8_t *data, size_t length);
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
    ~ItemsResponse() override;
    /// @}
private:
    class ItemsResponseImpl;
    std::unique_ptr<ItemsResponseImpl> pImpl;
};
}
#endif
