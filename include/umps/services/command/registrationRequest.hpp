#ifndef UMPS_SERVICES_COMMAND_REGISTRATION_REQUEST_HPP
#define UMPS_SERVICES_COMMAND_REGISTRATION_REQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::Command
{
 class ModuleDetails;
}
namespace UMPS::Services::Command
{
/// @class RegistrationRequest registrationRequest.hpp "umps/services/command/registrationRequest.hpp"
/// @brief Allows servers to request becoming a registered modules.
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
    /// @param[in,out] details  The request from which to initialize this class.
    ///                         On exit, details's behavior is undefined.
    RegistrationRequest(RegistrationRequest &&request) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] request  The request to copy to this.
    /// @result A deep copy of the request.
    RegistrationRequest& operator=(const RegistrationRequest &request);
    /// @brief Move assignment.
    /// @param[in,out] request  The request whose memory will be moved to this.
    ///                         On exit, request's behavior is undefined.
    /// @result The memory from request moved to this.
    RegistrationRequest& operator=(RegistrationRequest &&request) noexcept;
    /// @}

    /// @name Module Details (Required)
    /// @{

    /// @throws std::invalid_argument if details.haveName() is false.
    void setModuleDetails(const ModuleDetails &details);
    /// @result The module details.
    /// @throws std::runtime_error if \c haveModuleDetails() is false.
    [[nodiscard]] ModuleDetails getModuleDetails() const;
    /// @result True indicates the module details were set.
    [[nodiscard]] bool haveModuleDetails() const noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the request class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] std::string toMessage() const final;
    /// @brief Creates the class from a message.
    /// @param[in] message  The message.
    /// @throws std::runtime_error if the message is invalid.
    void fromMessage(const std::string &message) final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, size_t length) final;
    /// @result A message type indicating this is a text request message.
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @result The message version.
    [[nodiscard]] std::string getMessageVersion() const noexcept final;
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept final;
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