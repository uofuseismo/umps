#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLECONNECTIONSRESPONSE_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLECONNECTIONSRESPONSE_HPP
#include <memory>
#include <vector>
#include "umps/services/connectionInformation/enums.hpp"
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::ConnectionInformation
{
 class Details;
}
namespace UMPS::Services::ConnectionInformation
{
/// @class AvailableConnectionsResponse "availableConnectionsResponse.hpp" "umps/services/connectionInformation/availableConnectionsRepsonse.hpp"
/// @brief This class provides the connection information to for all
///        available connections - e.g., broadcasts, services, etc.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class AvailableConnectionsResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    AvailableConnectionsResponse();
    /// @brief Copy constructor.
    /// @param[in] response  Creates this class from the given response.
    AvailableConnectionsResponse(const AvailableConnectionsResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  Creates this class from the given response.
    ///                          On exit, response's behavior is undefined.
    AvailableConnectionsResponse(AvailableConnectionsResponse &&response) noexcept;
    /// @}

    /// @name Operator
    /// @{
    /// 
    /// @brief Copy assignment operator.
    /// @result A deep copy of the given response.
    AvailableConnectionsResponse& operator=(const AvailableConnectionsResponse &response);
    /// @brief Move asignment operator.
    /// @result The memory from response moved to this.
    AvailableConnectionsResponse& operator=(AvailableConnectionsResponse &&response) noexcept;
    /// @}

    /// @name Properties
    /// @{
    /// @brief Sets the connection details for every available broadcast,
    ///        service, etc.
    /// @throws std::invalid_argument if any of the details are not properly
    ///         specified.
    void setDetails(const std::vector<Details> &details);
    /// @result The connection details for every available broadcast, 
    ///         service, etc. 
    /// @note If the result is empty then there are no available connections.
    /// @sa \c getReturnCode().
    [[nodiscard]] std::vector<Details> getDetails() const noexcept;
    /// @brief Allows the incrementer to set its return code and signal to
    ///        the requester whether or not the increment was successful.
    /// @param[in] code   The return code.
    void setReturnCode(ReturnCode code) noexcept;
    /// @result The return code from the incrementer.
    [[nodiscard]] ReturnCode getReturnCode() const noexcept;
    /// @}

    /// @name Message Properties
    /// @{
    /// @brief Create a copy of this class.
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<IMessage> clone() const override final;
    /// @brief Create a clone of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept override final;
    /// @brief Converts this class to a string representation.
    /// @result The class expressed in string format.
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const override final;
    /// @brief Converts this message from a string representation to data.
    void fromMessage(const char *data, const size_t length) override final;
    /// @result The message type.
    [[nodiscard]] std::string getMessageType() const noexcept override final;
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
    /// @brief Reset class and release all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~AvailableConnectionsResponse() override;
    /// @}
private:
    class ResponseImpl;
    std::unique_ptr<ResponseImpl> pImpl; 

};
}
#endif
