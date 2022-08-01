#ifndef UMPS_SERVICES_MODULEREGISTRY_REGISTRATIONREQUEST_HPP
#define UMPS_SERVICES_MODULEREGISTRY_REGISTRATIONREQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::ModuleRegistry
{
/// @class RegistrationRequest registrationRequest.hpp "umps/services/moduleRegistry/registrationRequest.hpp"
/// @brief Requests a module be registered with UMPS.
/// @detail While no module needs to be explicitly registered, particularly 
///         read-only modules, this is a really good thing to do because
///         it allows remote shut down.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RegistrationRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RegistrationRequest();
    /// @brief Copy constructor.
    /// @param[in] request  The request from which to initialize this class.
    RegistrationRequest(const RegistrationRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] request  The request from which to initialize this class.
    ///                         On exit, request's behavior is undefined.
    RegistrationRequest(RegistrationRequest &&request) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] request  The request to copy to this.
    /// @result A deep copy of the input request.
    RegistrationRequest& operator=(const RegistrationRequest &request);
    /// @brief Move assignment operator.
    /// @param[in,out] request  The request whose memory will be moved to this.
    ///                         On exit, request's behavior is undefined.
    /// @result The memory from request moved to this.
    RegistrationRequest& operator=(RegistrationRequest &&request) noexcept;
    /// @}

    /// @name Required Properties
    /// @{

    /// @brief Sets the module name.
    /// @param[in] name   The module name.
    /// @throws std::invalid_argument if the name is empty.
    void setModuleName(const std::string &name);
    /// @result The module name.
    /// @throws std::runtime_error if the module name was not set.
    [[nodiscard]] std::string getModuleName() const;
    /// @result True indicates the module name was set.
    [[nodiscard]] bool haveModuleName() const noexcept;
    /// @}

    /// @name Optional Properties
    /// @{

    /// @brief Sets the machine on which the module is running.
    /// @param[in] machine  The machine name.
    void setMachine(const std::string &machine) noexcept;
    /// @result The machine on which the module is running.
    /// @note This can be blank.
    [[nodiscard]] std::string getMachine() const noexcept;

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
    ~RegistrationRequest();
    /// @}
private:
    class RegistrationRequestImpl;
    std::unique_ptr<RegistrationRequestImpl> pImpl;
};
}
#endif
