#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLECONNECTIONSREQUESTS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLECONNECTIONSREQUESTS_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::ConnectionInformation
{
/// @class AvailableConnectionsRequest "availableConnectionsRequest.hpp" "umps/services/connectionInformation/availableConnectionsRequest.hpp"
/// @brief This class requests the connection information for all avialable
///        connections - which includes services, broadcasts, etc.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class AvailableConnectionsRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    AvailableConnectionsRequest();
    /// @brief Copy constructor.
    /// @param[in] request  Creates this class from the given request.
    AvailableConnectionsRequest(const AvailableConnectionsRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] request  Creates this class from the given request.
    ///                         On exit, request's behavior is undefined.
    AvailableConnectionsRequest(AvailableConnectionsRequest &&request) noexcept;
    /// @}

    /// @name Operator
    /// @{
    /// 
    /// @brief Copy assignment operator.
    /// @result A deep copy of the given request.
    AvailableConnectionsRequest& operator=(const AvailableConnectionsRequest &request);
    /// @brief Move asignment operator.
    /// @result The memory from request moved to this.
    AvailableConnectionsRequest& operator=(AvailableConnectionsRequest &&request) noexcept;
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
    /// @brief Destructor.
    ~AvailableConnectionsRequest() override;
    /// @}
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl; 
};
}
#endif
