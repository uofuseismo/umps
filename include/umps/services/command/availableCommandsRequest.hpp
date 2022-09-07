#ifndef UMPS_SERVICES_COMMAND_AVAILABLECOMMANDSREQUEST_HPP
#define UMPS_SERVICES_COMMAND_AVAILABLECOMMANDSREQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::Command
{
/// @class AvailableCommandsRequest "availableCommandsRequest.hpp" "umps/messageFormats/availableCommandsRequest.hpp"
/// @brief Requests the text-based interactive program commands.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Messages_MessageFormats
class AvailableCommandsRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    AvailableCommandsRequest();
    /// @brief Copy constructor.
    /// @param[in] message  The text message class from which to initialize
    ///                     this class.
    AvailableCommandsRequest(const AvailableCommandsRequest &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The text message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    AvailableCommandsRequest(AvailableCommandsRequest &&message) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The text message class to copy to this.
    /// @result A deep copy of the text message.
    AvailableCommandsRequest& operator=(const AvailableCommandsRequest &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The text message class whose memory will be
    ///                         moved to this.  On exit, messages's behavior is
    ///                         undefined.
    /// @result The memory from message moved to this.
    AvailableCommandsRequest& operator=(AvailableCommandsRequest &&message) noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the class to a string message.
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
    /// @result A message type indicating the message type.
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
    ~AvailableCommandsRequest() override;
    /// @}
private:
    class AvailableCommandsRequestImpl;
    std::unique_ptr<AvailableCommandsRequestImpl> pImpl;
};
}
#endif
