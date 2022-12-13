#ifndef UMPS_PROXY_SERVICES_COMMAND_REGISTRATION_RESPONSE_HPP
#define UMPS_PROXY_SERVICES_COMMAND_REGISTRATION_RESPONSE_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::ProxyServices::Command
{
/// @brief The service's return code for a registration request.
enum class RegistrationReturnCode
{
    Success = 0,        /*!< The registration request succeeded. */
    InvalidRequest = 1, /*!< The registration rqeuest failed because the
                             request was invalid. */
    Exists = 2,         /*!< The registration request failed because the
                             module already exists. */
    ServerError = 3     /*!< The registration request failed because of a
                             server-side error. */
};
/// @class RegistrationResponse registrationResponse.hpp "umps/proxyServices/command/registrationResponse.hpp"
/// @brief The response to a module registration response.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @sa RegistrationResponse 
/// @ingroup UMPS_ProxyServices_Command
class RegistrationResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RegistrationResponse();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    RegistrationResponse(const RegistrationResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize this
    ///                          class.  On exit, details's behavior is
    ///                          undefined.
    RegistrationResponse(RegistrationResponse &&response) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] response  The response to copy to this.
    /// @result A deep copy of the response.
    RegistrationResponse& operator=(const RegistrationResponse &response);
    /// @brief Move assignment.
    /// @param[in,out] response  The response whose memory will be moved to this.
    ///                         On exit, response's behavior is undefined.
    /// @result The memory from response moved to this.
    RegistrationResponse& operator=(RegistrationResponse &&response) noexcept;
    /// @}

    /// @name Return Code
    /// @{

    /// @brief Sets the registration request return code.
    /// @param[in] code  The return code.
    void setReturnCode(RegistrationReturnCode code) noexcept;
    /// @result The registration return code.
    [[nodiscard]] RegistrationReturnCode getReturnCode() const;
    /// @result True indicates the return code was set.
    [[nodiscard]] bool haveReturnCode() const noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the response class to a string message.
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
    /// @result A message type indicating this is a text response message.
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
    ~RegistrationResponse();
    /// @}
private:
    class RegistrationResponseImpl;
    std::unique_ptr<RegistrationResponseImpl> pImpl;
};
}
#endif
