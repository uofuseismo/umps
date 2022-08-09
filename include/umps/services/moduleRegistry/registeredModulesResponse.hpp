#ifndef UMPS_SERVICES_MODULEREGISTRY_REGISTEREDMODULESRESPONSE_HPP
#define UMPS_SERVICES_MODULEREGISTRY_REGISTEREDMODULESRESPONSE_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
#include "umps/services/moduleRegistry/enums.hpp"
namespace UMPS::Services::ModuleRegistry
{
 class ModuleDetails;
}
namespace UMPS::Services::ModuleRegistry
{
/// @class RegisteredModulesResponse registeredModulesResponse.hpp "umps/services/moduleRegistry/registeredModulesResponse.hpp"
/// @brief The modules registered with UMPS.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RegisteredModulesResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RegisteredModulesResponse();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    RegisteredModulesResponse(const RegisteredModulesResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize this class.
    ///                         On exit, response's behavior is undefined.
    RegisteredModulesResponse(RegisteredModulesResponse &&response) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] response  The response to copy to this.
    /// @result A deep copy of the input response.
    RegisteredModulesResponse& operator=(const RegisteredModulesResponse &response);
    /// @brief Move assignment operator.
    /// @param[in,out] response  The response whose memory will be moved to this.
    ///                         On exit, response's behavior is undefined.
    /// @result The memory from response moved to this.
    RegisteredModulesResponse& operator=(RegisteredModulesResponse &&response) noexcept;
    /// @}

    /// @name Required Properties
    /// @{

    /// @brief Sets the registered module details.
    /// @param[in] modules  The registered modules.
    /// @throws std::invalid_argument if any of the modules do not have a name.
    void setModules(const std::vector<ModuleDetails> &modules);
    /// @result The registered modules.
    [[nodiscard]] std::vector<ModuleDetails> getModules() const noexcept;

    /// @brief Sets the return code.
    /// @param[in] code  The return code.
    void setReturnCode(RegisteredModulesReturnCode code) noexcept;
    /// @result The return code.
    /// @throws std::runtime_error if \c haveReturnCode() is false.
    [[nodiscard]] RegisteredModulesReturnCode getReturnCode() const;
    /// @result True indicates the return code was set.
    [[nodiscard]] bool haveReturnCode() const noexcept;
    /// @}

    /// @name Optional Properties
    /// @{

    /// @brief For asynchronous messaging this allows the responder to index
    ///        the response.  This value will be returned so the responder
    ///        can track which response was filled by the response.
    /// @param[in] identifier   The response identifier.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result The response identifier.
    [[nodiscard]] uint64_t getIdentifier() const noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An instance of an uninitialized class.
    [[nodiscard]] virtual std::unique_ptr<IMessage> createInstance() const noexcept override final;
    /// @brief Converts the response class to a string message.
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

    /// @brief Creates the class from a JSON response message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the response class to a JSON message.  This is useful
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
    /// @brief Converts the response class to a CBOR message.
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
    ~RegisteredModulesResponse();
    /// @}
private:
    class RegisteredModulesResponseImpl;
    std::unique_ptr<RegisteredModulesResponseImpl> pImpl;
};
}
#endif
