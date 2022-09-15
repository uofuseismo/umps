#ifndef UMPS_SERVICES_COMMAND_TERMINATERESPONSE_HPP
#define UMPS_SERVICES_COMMAND_TERMINATERESPONSE_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::Command
{
/// @brief The services return code accompanying the response.
enum class TerminateReturnCode
{
    Success = 0,         /*!< Indicates the command was successful. */
    InvalidCommand = 1,  /*!< The command is invalid. */
    ApplicationError = 2 /*!< The terminate request is valid but the
                              application could not process the request. */
};
/// @class TerminateResponse terminateResponse.hpp "umps/services/command/quitResponse.hpp"
/// @brief A request to terminate the application.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class TerminateResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    TerminateResponse();
    /// @brief Copy constructor.
    /// @param[in] message  The text request message class from which to
    ///                     initialize this class.
    TerminateResponse(const TerminateResponse &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The text request message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    TerminateResponse(TerminateResponse &&message) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The message class to copy to this.
    /// @result A deep copy of the message.
    TerminateResponse& operator=(const TerminateResponse &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The response message class whose memory will
    ///                         be moved to this.  On exit, messages's behavior
    ///                         is undefined.
    /// @result The memory from message moved to this.
    TerminateResponse& operator=(TerminateResponse &&message) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief Sets the return code.
    /// @param[in] code  The return code.
    void setReturnCode(TerminateReturnCode code) noexcept;
    /// @result The return code.
    [[nodiscard]] TerminateReturnCode getReturnCode() const;
    /// @result True indicates the return code is set.
    [[nodiscard]] bool haveReturnCode() const noexcept;
    /// @}


    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the text request class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const final;
    /// @brief Creates the class from a message.
    /// @param[in] message  The message.
    /// @throws std::runtime_error if the message is invalid.
    void fromMessage(const std::string &message);
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, size_t length) final;
    /// @result A message type indicating this is a text request message.
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept final;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~TerminateResponse() override;
    /// @}
public:
    class TerminateResponseImpl;
    std::unique_ptr<TerminateResponseImpl> pImpl;
};
}
#endif
