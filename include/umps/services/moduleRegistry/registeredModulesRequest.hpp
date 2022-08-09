#ifndef UMPS_SERVICES_MODULEREGISTRY_REGISTEREDMODULESREQUEST_HPP
#define UMPS_SERVICES_MODULEREGISTRY_REGISTEREDMODULESREQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::ModuleRegistry
{
/// @class RegisteredModulesRequest registeredModulesRequest.hpp "umps/services/moduleRegistry/registeredModulesRequest.hpp"
/// @brief Requests the list of modules registered with UMPS.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RegisteredModulesRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RegisteredModulesRequest();
    /// @brief Copy constructor.
    /// @param[in] request  The request from which to initialize this class.
    RegisteredModulesRequest(const RegisteredModulesRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] request  The request from which to initialize this class.
    ///                         On exit, request's behavior is undefined.
    RegisteredModulesRequest(RegisteredModulesRequest &&request) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] request  The request to copy to this.
    /// @result A deep copy of the input request.
    RegisteredModulesRequest& operator=(const RegisteredModulesRequest &request);
    /// @brief Move assignment operator.
    /// @param[in,out] request  The request whose memory will be moved to this.
    ///                         On exit, request's behavior is undefined.
    /// @result The memory from request moved to this.
    RegisteredModulesRequest& operator=(RegisteredModulesRequest &&request) noexcept;
    /// @}

    /// @name Optional Properties
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
    [[nodiscard]] virtual std::unique_ptr<IMessage> createInstance() const noexcept override final;
    /// @brief Converts the request class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] virtual std::string toMessage() const override final;
    /// @brief Creates the class from a message.
    /// @param[in] message  The message in a string container.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if the message is empty.
    void fromMessage(const std::string &message);
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    virtual void fromMessage(const char *data, size_t length) override final;
    /// @result The message type.
    [[nodiscard]] virtual std::string getMessageType() const noexcept final;
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
    /// @brief Converts the request class to a CBOR message.
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
    ~RegisteredModulesRequest();
    /// @}
private:
    class RegisteredModulesRequestImpl;
    std::unique_ptr<RegisteredModulesRequestImpl> pImpl;
};
}
#endif
