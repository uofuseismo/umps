#ifndef UMPS_SERVICES_INCREMENTER_REQUEST_HPP
#define UMPS_SERVICES_INCREMENTER_REQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
#include "umps/services/incrementer/enums.hpp"
namespace UMPS::Services::Incrementer
{
/// @class Request "request.hpp" "umps/services/incrementer/request.hpp"
/// @brief To identify items (picks, events, origins, amplitudes, etc.) in the
///        processing pipeline we must assign them a unique object identifier.
///        The challenge in a distributed setting is that we need a single
///        entity that performs a lock-increment-unlock operation.  This is the
///        mechanism for requesting an identifier from that entity. 
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Request : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Request();
    /// @brief Copy constructor.
    /// @param[in] request   The request from which to initialize this class.
    Request(const Request &request);
    /// @brief Move constructor.
    /// @param[in,out] request  The request from which to initialize this class.
    ///                         On exit, request's behavior is undefined.
    Request(Request &&request) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] request   The request to copy to this.
    /// @result A deep copy of the the input request.
    Request& operator=(const Request &request);
    /// @brief Move assignment operator.
    /// @param[in,out] request  The request whose memory will be moved to this.
    ///                         On exit, request's behavior is undefined.
    /// @result The memory from request moved to this.
    Request& operator=(Request &&request) noexcept;
    /// @}

    /// @name Required Information
    /// @{
    /// @brief This represents a custom item to be incremented.
    /// @param[in] item  The item that (e.g., table name) that we are
    ///                  requesting be incremented.   
    /// @throws std::invalid_argument if the item is empty.
    void setItem(const std::string &item);
    /// @brief This defines a standard item to be incremented.
    /// @param[in] item  The item whose identifier we are requesting be
    ///                  incremented.
    void setItem(Item item) noexcept;
    /// @result The item identifier.
    [[nodiscard]] std::string getItem() const;
    /// @result True indicates that the item was set.
    [[nodiscard]] bool haveItem() const noexcept;
    /// @}

    /// @name Optional Information
    /// @{
    /// @brief For asynchronous messaging this allows the requester to index
    ///        the request.  This value will be returned so the requester
    ///        can track which request was filled by the response.
    /// @param[in] identifier   The request identifier.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result The request identifier.
    [[nodiscard]] uint64_t getIdentifier() const noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An instance of an uninitialized class.
    virtual std::unique_ptr<IMessage> createInstance() const noexcept override final;
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
    void fromCBOR(const uint8_t *data, const size_t length) override final;
    /// @brief Converts the packet class to a CBOR message.
    /// @result The class expressed in Compressed Binary Object Representation
    ///         (CBOR) format.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] std::string toCBOR() const override final;
    /// @result The message type - e.g., "DataPacket".
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @}

    /// @name Debugging Utilities
    /// @{
    /// @brief Creates the class from a JSON request message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the request class to a JSON message.  This is useful
    ///        for debugging.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @}
 
    /// @name Destructors
    /// @{
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~Request();
    /// @}
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl;
};
}
#endif
