#ifndef UMPS_PYTHON_MESSAGE_FORMATS_HPP
#define UMPS_PYTHON_MESSAGE_FORMATS_HPP
#include <memory>
#include <vector>
#include <string>
#include <pybind11/pybind11.h>
namespace UMPS::MessageFormats
{
 class IMessage;
 class Messages; 
 class Failure;
 class Text;
}
namespace UMPS::Python::MessageFormats
{
/// @class IMessage
/// @brief Defines some add-ons that allow Python to interact with UMPS
///        IMessage-dervied messages.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IMessage
{
public:
    /// @brief Destructor.
    virtual ~IMessage();
    /// @brief Creates this base class from an UMPS message.
    /// @note This does nothing.
    virtual void fromBaseClass(UMPS::MessageFormats::IMessage &message);
    /// @result The message type.
    [[nodiscard]] virtual std::string getMessageType() const noexcept;
    /// @brief Returns a copy of this class.
    /// @result A null pointer.
    [[nodiscard]] virtual std::unique_ptr<IMessage> clone(const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const;
    /// @brief Returns an instance of this class.
    /// @result A null pointer.
    [[nodiscard]] virtual std::unique_ptr<IMessage> createInstance() const;
    /// @brief Returns a pointer to the base class.
    /// @result A null pointer.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> getInstanceOfBaseClass() const noexcept;
};
/// @class PyMessage
/// @brief This is a specialized class that allows the user to create a message
///        in Python.
/*
class PyMessage : public UMPS::MessageFormats::IMessage, IMessage
{
public:
    void setMessageType(const std::string &messageType);
    [[nodiscard]] getMessageType() const noexcept override;
private:

};
*/
/// @class Messages
/// @brief A container for Python IMessage types.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Messages
{
public:
    /// @brief C'tor
    Messages();
    /// @brief Copy constructor.
    Messages(const Messages &messages);
    /// @brief Constructs from the UMPS messages class.
    explicit Messages(const UMPS::MessageFormats::Messages &messages);
    /// @brief Move constructor.
    Messages(Messages &&messages) noexcept;
    /// @brief Destructor.
    ~Messages();
    /// @result A copy of the underlying native class. 
    [[nodiscard]] UMPS::MessageFormats::Messages getNativeClass() const noexcept;
    /// @result Copy assignment operator.
    Messages& operator=(const Messages &messages);
    /// @result Copy assignment operator from the UMPS base class.
    Messages& operator=(const UMPS::MessageFormats::Messages &messages);
    /// @result MOve assignment operator.
    Messages& operator=(Messages &&messages) noexcept;

    /// @brief Adds a message type to the class.
    void add(const IMessage &message);
    /// @result The number of messages in the class.
    [[nodiscard]] int size() const noexcept;
    /// @result True indicates there are no message formats.
    [[nodiscard]] bool empty() const noexcept;

    //[[nodiscard]] std::unique_ptr<IMessage> get(const std::string &messageType) const;
    //[[nodiscard]] std::vector<std::unique_ptr<IMessage>> getPythonMessageTypes() const;
    //[[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> getBaseClass() const noexcept = 0;
private:
    std::unique_ptr<UMPS::MessageFormats::Messages> pImpl;
    //std::vector<std::unique_ptr<IMessage>> mPythonMessageTypes;
};
/// @class Failure
/// @brief A generic failure message that can be returned from a service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Failure : public IMessage
{
public:
    /// @brief Constructor.
    Failure();
    /// @brief Copy constructor.
    Failure(const Failure &failure);
    /// @brief Constructs from the base class.
    explicit Failure(const UMPS::MessageFormats::Failure &failure);
    /// @brief Move constructor.
    Failure(Failure &&failure) noexcept;
    /// @brief Copy assignment operator.
    Failure& operator=(const Failure &);
    /// @brief Copy of this class from a base class.
    Failure& operator=(const UMPS::MessageFormats::Failure &failure);
    /// @brief Move assignment operator.
    Failure& operator=(Failure &&) noexcept;
    /// @brief Creates the message from a base class.
    void fromBaseClass(UMPS::MessageFormats::IMessage &message) override;
    /// @result A copy of the native class.
    [[nodiscard]] UMPS::MessageFormats::Failure getNativeClass() const noexcept;
    /// @brief An instance of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const override;
    /// @brief A copy of this class.
    [[nodiscard]] std::unique_ptr<IMessage> clone(const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const override;
    /// @brief A clone of the base class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> getInstanceOfBaseClass() const noexcept override;
    /// @result The message type.
    [[nodiscard]] std::string getMessageType() const noexcept override;
    /// @brief Sets the failure message details.
    void setDetails(const std::string &details) noexcept;
    /// @result The failure mesasge details.
    [[nodiscard]] std::string getDetails() const noexcept;
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor. 
    ~Failure() override;
private:
    std::unique_ptr<UMPS::MessageFormats::Failure> pImpl;
};
/// @class Text
/// @brief A generic text message.  Generally speaking, you should not be using
///        this class for messaging.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Text : public IMessage
{
public:
    /// @brief Constructor.
    Text();
    /// @brief Copy constructor.
    Text(const Text &failure);
    /// @brief Constructs from the base class.
    explicit Text(const UMPS::MessageFormats::Text &text);
    /// @brief Move constructor.
    Text(Text &&text) noexcept;
    /// @brief Copy assignment operator.
    Text& operator=(const Text &); 
    /// @brief Copy of this class from a base class.
    Text& operator=(const UMPS::MessageFormats::Text &text);
    /// @brief Move assignment operator.
    Text& operator=(Text &&) noexcept;
    /// @brief Creates the message from a base class.
    void fromBaseClass(UMPS::MessageFormats::IMessage &message) override;
    /// @result A copy of the native class.
    [[nodiscard]] UMPS::MessageFormats::Text getNativeClass() const noexcept;
    /// @brief An instance of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const override;
    /// @brief A copy of this class.
    [[nodiscard]] std::unique_ptr<IMessage> clone(const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const override;
    /// @brief Clone of the base class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> getInstanceOfBaseClass() const noexcept override;
    /// @result The message type.
    [[nodiscard]] std::string getMessageType() const noexcept override;
    /// @brief Sets the contents of the message.
    void setContents(const std::string &contents) noexcept;
    /// @result The contents of the message.
    [[nodiscard]] std::string getContents() const noexcept;
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor. 
    ~Text() override;
private:
    std::unique_ptr<UMPS::MessageFormats::Text> pImpl;
};
void initialize(pybind11::module &m);
}
#endif
