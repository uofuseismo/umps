#ifndef UMPS_SERVICES_COMMAND_COMMAND_RESPONSE_HPP
#define UMPS_SERVICES_COMMAND_COMMAND_RESPONSE_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
#include "umps/services/command/enums.hpp"
namespace UMPS::Services::Command
{
/// @class CommandResponse commandResponse.hpp "umps/services/command/commandResponse.hpp"
/// @brief The response from a text-based command.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_uLocalCommand
class CommandResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    CommandResponse();
    /// @brief Copy constructor.
    /// @param[in] message  The text request message class from which to
    ///                     initialize this class.
    CommandResponse(const CommandResponse &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The text request message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    CommandResponse(CommandResponse &&message) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief The text-based response to the command.
    /// @param[in] response  The text-based response.
    /// @throws std::invalid_argument if the response is empty.
    void setResponse(const std::string &response);
    /// @result The text-based response.
    /// @throws std::runtime_error if \c haveResponse() is false.
    [[nodiscard]] std::string getResponse() const;
    /// @result True indicates the response was set.
    [[nodiscard]] bool haveResponse() const noexcept;

    /// @brief Sets the return code.
    /// @param[in] code  The return code.
    void setReturnCode(CommandReturnCode code) noexcept;
    /// @result The return code.
    [[nodiscard]] CommandReturnCode getReturnCode() const;
    /// @result True indicates the return code is set.
    [[nodiscard]] bool haveReturnCode() const noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The text request message class to copy to this.
    /// @result A deep copy of the text message.
    CommandResponse& operator=(const CommandResponse &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The text request message class whose memory will
    ///                         be moved to this.  On exit, messages's behavior
    ///                         is undefined.
    /// @result The memory from message moved to this.
    CommandResponse& operator=(CommandResponse &&message) noexcept;
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

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~CommandResponse() override;
    /// @}
private:
    class CommandResponseImpl;
    std::unique_ptr<CommandResponseImpl> pImpl;
};
}
#endif
