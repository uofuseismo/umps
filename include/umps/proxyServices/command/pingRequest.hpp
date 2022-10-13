#ifndef UMPS_PROXY_SERVICES_COMMAND_PING_REQUEST_HPP
#define UMPS_PROXY_SERVICES_COMMAND_PING_REQUEST_HPP
#include <memory>
#include <chrono>
#include "umps/messageFormats/message.hpp"
namespace UMPS::ProxyServices::Command
{
/// @class PingRequest pingRequest.hpp "umps/proxyServices/command/pingRequest.hpp"
/// @brief Specialized request between the proxy and replier to indicate the
///        module is still running.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class PingRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    PingRequest();
    /// @brief Copy constructor.
    /// @param[in] request  The request from which to initialize this class.
    PingRequest(const PingRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] details  The request from which to initialize this class.
    ///                         On exit, details's behavior is undefined.
    PingRequest(PingRequest &&request) noexcept;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the request class to a string message.
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

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~PingRequest();
    /// @}
private:
    class PingRequestImpl;
    std::unique_ptr<PingRequestImpl> pImpl;
};
}
#endif
