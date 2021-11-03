#ifndef UMPS_MESSAGEFORMATS_MESSAGES_HPP
#define UMPS_MESSAGEFORMATS_MESSAGES_HPP 
#include <map>
#include <memory>
namespace UMPS::MessageFormats
{
 class IMessage;
}
namespace UMPS::MessageFormats
{
/// @brief This is a container for holding multiple (unique) message formats.
/// @copyright Ben Baker (University of Utah)
class Messages
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Messages();
    /// @brief Copy constructor.
    /// @param[in] messages  The messages container from which to initialize
    ///                      this class.
    Messages(const Messages &messages);
    /// @brief Move constructor.
    /// @param[in,out] messages  The messages container from which to initialize
    ///                          this class.  On exit, messages's behavior is
    ///                          undefined.
    Messages(Messages &&messages) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] messages  The messages container to copy to this.
    /// @result A deep copy of the input messages container.
    Messages& operator=(const Messages &messages);
    /// @brief Move assignment operator.
    /// @param[in,out] messages  The messages container whose memory will be
    ///                          moved to this.  On exit, messageses's behavior
    ///                          is undefined.
    Messages& operator=(Messages &&messages) noexcept;
    /// @}

    /// @brief Add the message type to the container.
    /// @param[in] message   The message to add to the container.
    /// @throws std::invalid_argument if the message type already exists.
    void add(const std::unique_ptr<IMessage> &message);

    /// @param[in] message  Determines if the message is set b calling
    ///                     \c contains(getMessageType()).
    /// @result True indicates the message type exists.
    /// @throws std::invalid_argument if the message is NULL.
    [[nodiscard]] bool contains(const std::unique_ptr<IMessage> &message) const;
    /// @param[in] messageType   The name of the message type.
    /// @result True indicates the message type exists.
    [[nodiscard]] bool contains(const std::string &messageType) const noexcept;

    /// @brief Removes the message.
    /// @throws std::invalid_argument if the message is NULL.
    void remove(const std::unique_ptr<IMessage> &message);
    /// @param[in] messageType  The name of the message type to remove.
    /// @throws std::runtime_error if \c contains(messageType) is false.
    void remove(const std::string &messageType);

    /// @result The number of message formats.
    [[nodiscard]] int size() const noexcept;
    /// @result True indicates there are no messages.
    [[nodiscard]] bool empty() const noexcept;

    /// @result All the message types in this container.
    std::map<std::string, std::unique_ptr<IMessage>> get() const noexcept;
    /// @result The message container corresponding to the message type,
    std::unique_ptr<IMessage> get(const std::string &messageType) const;

    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~Messages();
    /// @}
private:
    class MessagesImpl;
    std::unique_ptr<MessagesImpl> pImpl;
};
}
#endif
