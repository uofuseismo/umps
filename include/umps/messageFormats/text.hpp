#ifndef UMPS_MESSAGEFORMATS_GENERIC_HPP
#define UMPS_MESSAGEFORMATS_GENERIC_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::MessageFormats
{
/// @class Text "text.hpp" "umps/messageFormats/text.hpp"
/// @brief Defines a text-based message.  For example, this class would allow
///        you to send the contents of a text file.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Text : public IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Text();
    /// @brief Copy constructor.
    /// @param[in] message  The text message class from which to initialize
    ///                     this class.
    Text(const Text &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The text message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    Text(Text &&message) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The text message class to copy to this.
    /// @result A deep copy of the text message.
    Text& operator=(const Text &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The text message class whose memory will be
    ///                         moved to this.  On exit, messages's behavior is
    ///                         undefined.
    /// @result The memory from message moved to this.
    Text& operator=(Text &&message) noexcept;
    /// @}

    /// @brief Sets the contents of a text message.
    /// @param[in] text  The text to set.
    void setContents(const std::string &contents) noexcept;
    /// @result The contents of the text message.
    [[nodiscard]] std::string getContents( ) const noexcept;

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the pick class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const override final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, size_t length) override final;
    /// @result A message type indicating this is a pick message.
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
    ~Text() override;
    /// @}
private:
    class TextImpl;
    std::unique_ptr<TextImpl> pImpl;
};
}
#endif
