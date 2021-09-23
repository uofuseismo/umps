#ifndef UMPS_MESSAGING_REQUESTROUTER_ROUTER_HPP
#define UMPS_MESSAGING_REQUESTROUTER_ROUTER_HPP
#include <memory>
#include <functional>
// Forward declarations
namespace UMPS
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
namespace UMPS::Messaging::RequestRouter
{
/// @class Router "router.hpp" "umps/messaging/requestRouter/router.hpp"
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
    /// @brief Constructs a router with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Router(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a router with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Router(std::shared_ptr<zmq::context_t> &context);
    /// @}

    /// @param[in] endPoint  The endpoint for accepting connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @param[in] callBack  The callback function.  This creates a response
    ///                      message for the requested message.
//    void initialize(const std::string &endPoint,
//                    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
//                                        (const UMPS::MessageFormats::IMessage *)> &callback);
    /// @brief Adds a message subscription type.
 
    /// @param[in] endpoint  The endpoint for accepting connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @note For more see: http://api.zeromq.org/2-1:zmq-bind. 
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endpoint);
    /// @result True indicates that the router is bound to an address.
    [[nodiscard]] bool isBound() const noexcept;
    /// @brief Sets the callback function.
    /// @param[in] callback  The callback function.  This creates a response
    ///                      message for the requested message.
    /// @note This function should not throw an exception and should always
    ///       return something.
//    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
//                                        (const UMPS::MessageFormats::IMessage *)> &callback);
    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                         (const std::string &, const uint8_t *, size_t)> &callback);
    /// @result True indicates that the callback was set.
    [[nodiscard]] bool haveCallback() const noexcept;
    /// @result True indicates that the class is initialized.
//    [[nodiscard]] bool isInitialized() const noexcept;

    /// @result True the router is connected.
//    [[nodiscard]] bool isConnected() const noexcept;

    /// @brief Adds a subscription to message of type message.getMessagetype();
    /// @param[in] message  The message type to which the router will subscribe.
    /// @throws std::runtime_error \c isConnected() is false.
    void addMessageType(std::unique_ptr<UMPS::MessageFormats::IMessage> &message);

    /// @throws std::runtime_error() if \c isInitialized() is false.
    void start();
    /// @result True indicates that the service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Interrupts the thread.
    void stop();

    /// @name Destructors
    /// @{
    ~Router();
    /// @}

    void operator()();

    Router& operator=(const Router &router) = delete;
    Router& operator=(Router &&router) = delete;
private:
    class RouterImpl;
    std::unique_ptr<RouterImpl> pImpl;
};
}
#endif
