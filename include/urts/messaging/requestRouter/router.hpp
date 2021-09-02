#ifndef URTS_MESSAGING_REQUESTROUTER_ROUTER_HPP
#define URTS_MESSAGING_REQUESTROUTER_ROUTER_HPP
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
/// @class Router "router.hpp" "urts/messaging/requestRouter/router.hpp"
/// @brief A ZeroMQ router for use in the request-router combination.
/// @detail The request-router pattern is useful for when we want an 
///         asynchronous server that can talk to multiple request clients
///         at the same time.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Router
{
public:
    /// @name Constructors
    /// @{
    Router();
    /// @}

    /// @param[in] endPoint  The endpoint for accepting connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @param[in] callBack  The callback function.  This creates a response
    ///                      message for the requested message.
    void initialize(const std::string &endPoint,
                    const std::function<std::unique_ptr<URTS::MessageFormats::IMessage>
                                        (const URTS::MessageFormats::IMessage *)> &callback);
    /// @brief Adds a message subscription type.
 
    /// @result True indicates that the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;

    /// @result True the router is connected.
    [[nodiscard]] bool isConnected() const noexcept;

    /// @brief Adds a subscription to message of type message.getMessagetype();
    /// @param[in] message  The message type to which the router will subscribe.
    /// @throws std::runtime_error \c isConnected() is false.
    void addSubscription(std::unique_ptr<URTS::MessageFormats::IMessage> &message);

    /// @throws std::runtime_error() if \c isInitialized() is false.
    void start();
    /// @result True indicates that the service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Interrupts the thread.
    void stop();
    /// @param[in] endPoint  The endpoint for accepting connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @note For more see: http://api.zeromq.org/2-1:zmq-bind. 
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint);

    /// @name Destructors
    /// @{
    ~Router();
    /// @}
private:
    class RouterImpl;
    std::unique_ptr<RouterImpl> pImpl;
};
}
#endif
