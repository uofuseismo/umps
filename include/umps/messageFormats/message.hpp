#ifndef UMPS_MESSAGEFORMATS_MESSAGE_HPP
#define UMPS_MESSAGEFORMATS_MESSAGE_HPP
namespace UMPS::MessageFormats
{
/// @class IMessage "message.hpp" "umps/messageFormats/message.hpp"
/// @brief An abstract base class defining the base requirements a message
///        format must satisfy.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Messages_BaseClass
class IMessage
{
public:
    /// @brief Destructor.
    virtual ~IMessage() = default;
    /// @brief Create a copy of this class.
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<IMessage> clone() const = 0;
    /// @brief Create a clone of this class.
    [[nodiscard]] virtual std::unique_ptr<IMessage> createInstance() const noexcept = 0;
    /// @brief Converts this class to a string representation.
    /// @result The class expressed in string format.
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] virtual std::string toMessage() const = 0;
    /// @brief Converts this message from a string representation to data.
    virtual void fromMessage(const char *data, const size_t length) = 0;
    /// @result The message type.
    [[nodiscard]] virtual std::string getMessageType() const noexcept = 0;
};
}
#endif
