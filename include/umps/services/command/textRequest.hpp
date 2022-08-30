#ifndef UMPS_SERVICES_COMMAND_TEXTREQUEST_HPP
#define UMPS_SERVICES_COMMAND_TEXTREQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::Command
{
/// @class TextRequest textRequest.hpp "umps/services/command/textRequest.hpp"
/// @brief Issues a text-based command to the application.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class TextRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    TextRequest();
    /// @brief Copy constructor.
    /// @param[in] message  The text request message class from which to
    ///                     initialize this class.
    TextRequest(const TextRequest &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The text request message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    TextRequest(TextRequest &&message) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief The text-based command.
    /// @param[in] command  The text-based command. 
    /// @throws std::invalid_argument if the command is empty.
    void setCommand(const std::string &command);
    /// @result The text-based command.
    /// @throws std::runtime_error if \c haveCommand() is false.
    [[nodiscard]] std::string getCommand() const;
    /// @result The 
    [[nodiscard]] bool haveCommand() const noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The text request message class to copy to this.
    /// @result A deep copy of the text message.
    TextRequest& operator=(const TextRequest &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The text request message class whose memory will
    ///                         be moved to this.  On exit, messages's behavior
    ///                         is undefined.
    /// @result The memory from message moved to this.
    TextRequest& operator=(TextRequest &&message) noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the text request class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const override final;
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
    void fromMessage(const char *data, size_t length) override final;
    /// @result A message type indicating this is a text request message.
    [[nodiscard]] std::string getMessageType() const noexcept override final;
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept override final;
    /// @}
    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~TextRequest() override;
    /// @}
private:
    class TextRequestImpl;
    std::unique_ptr<TextRequestImpl> pImpl;
};
}
#endif
