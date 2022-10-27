#ifndef UMPS_MESSAGE_FORMATS_FAILURE_HPP
#define UMPS_MESSAGE_FORMATS_FAILURE_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::MessageFormats
{
/// @class Failure "failure.hpp" "umps/messageFormats/failure.hpp"
/// @brief This message is a general failure message.  For generalized
///        request-reply mechanisms, the server side code cannot terminate
///        with an exception.  In these truly exceptional situations, this
///        generic failure message can be returned.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Messages_MessageFormats
class Failure : public IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Failure();
    /// @brief Copy constructor.
    /// @param[in] message  The failure message class from which to initialize
    ///                     this class.
    Failure(const Failure &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The failure message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    Failure(Failure &&message) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The failure message class to copy to this.
    /// @result A deep copy of the failure message.
    Failure& operator=(const Failure &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The failure message class whose memory will be
    ///                         moved to this.  On exit, messages's behavior is
    ///                         undefined.
    /// @result The memory from message moved to this.
    Failure& operator=(Failure &&message) noexcept;
    /// @}

    /// @brief Sets the details of a failure message.
    /// @param[in] details  The failure details to set.
    void setDetails(const std::string &details) noexcept;
    /// @result The failure details.
    [[nodiscard]] std::string getDetails() const noexcept;

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the failure class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const final;
    /// @breif Creates the class from a message.
    /// @param[in] message  The contents of the message.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data.empty() is true.
    void fromMessage(const std::string &message) final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    void fromMessage(const char *data, size_t length) final;
    /// @result A message type indicating this is a failure message.
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
    ~Failure() override;
    /// @}
private:
    class FailureImpl;
    std::unique_ptr<FailureImpl> pImpl;
};
}
#endif
