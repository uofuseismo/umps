#ifndef URTS_MESSAGING_REQUESTROUTER_REQUEST_HPP
#define URTS_MESSAGING_REQUESTROUTER_REQUEST_HPP
#include <memory>
#include <functional>
// Forward declarations
namespace URTS
{
 namespace Logging
 {
  class ILog;
 }
 namespace MessageFormats
 {
  class IMessage;
 }
}
namespace zmq
{
 class context_t;
}
namespace URTS::Messaging::RequestRouter
{
/// @class Request "request.hpp" "urts/messaging/requestRouter/request.hpp"
/// @brief A ZeroMQ request for use in the request-router combination.
/// @detail The request-router pattern is useful for when we want an 
///         asynchronous server that can talk to multiple request clients
///         at the same time.  This class represents the client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Request
{
public:
    /// @name Constructors
    /// @{
    Request();
    /// @brief Constructs a request with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Request(std::shared_ptr<URTS::Logging::ILog> &logger);
    /// @brief Constructs a request with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Request(std::shared_ptr<zmq::context_t> &context);
    /// @}

    /// @brief Sets the message format for receiving responses. 
    void setResponse(std::unique_ptr<URTS::MessageFormats::IMessage> &message);
    void setCallback(const std::function<std::unique_ptr<URTS::MessageFormats::IMessage>
                                         (const std::string &, const uint8_t *, size_t)> &callback);

    /// @brief Connects to the given address.
    /// @param[in] endpoint  The endpoint to which to connect.
    void connect(const std::string &endpoint);
    /// @result True indicates that the requester is connected.
    [[nodiscard]] bool isConnected() const noexcept;

    /// @brief Performs a blocking request of from the router.
    /// @param[in] request  The request to make to the server via the router. 
    /// @result The response to the request from the server (via the router).
    [[nodiscard]]
    std::unique_ptr<URTS::MessageFormats::IMessage> request(
        const MessageFormats::IMessage &request);

    /// @brief Disconnects.
    void disconnect(); //const std::string &endpoint);

    /// @name Destructors
    /// @{
    ~Request();
    /// @}
private:
    class RequestImpl;
    std::unique_ptr<RequestImpl> pImpl;
};
}
#endif
