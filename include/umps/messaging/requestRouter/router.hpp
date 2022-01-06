#ifndef UMPS_MESSAGING_REQUESTROUTER_ROUTER_HPP
#define UMPS_MESSAGING_REQUESTROUTER_ROUTER_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
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
 namespace Messaging::RequestRouter
 {
  class RouterOptions;
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
/// @details The request-router pattern is useful for when we want an 
///          asynchronous server that can talk to multiple request clients
///          at the same time.
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
    /// @brief Constructs a router socket with the given context and logger.
    Router(std::shared_ptr<zmq::context_t> &context,
           std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}
 
    /// @name Step 1: Initialization
    /// @brief Initializes the router.
    /// @param[in] options   The router options.
    /// @throws std::invalid_argument if the endpoint or callback is not set. 
    void initialize(const RouterOptions &options);
    /// @result The security level of the connection.
    [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept;
    /// @result True indicates that the router is bound to an address,
    ///         has a callback, and is ready to receive and process messages.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result Gets the connection string.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] std::string getEndPoint() const;
    /// @}

    /// @name Step 2: Start the Router
    /// @{
    /// @brief Starts the router.  The router will receive requests,
    ///        process the reqeusts, and return an answer.  The processing
    ///        is defined by the callback.
    /// @throws std::runtime_error() if \c isInitialized() is false.
    void start();
    /// @result True indicates that the service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @}

    /// @name Step 3: Stop the Router
    /// @{
    /// @brief Interrupts the thread running the router.
    void stop();
    /// @}

    /// @name Destructors
    /// @{
    ~Router();
    /// @}

    void operator()();

    Router(const Router &router) = delete;
    Router(Router &&router) noexcept = delete; // Bad stuff happens to move and threads
    Router& operator=(const Router &router) = delete;
    Router& operator=(Router &&router) noexcept = delete;
private:
    class RouterImpl;
    std::unique_ptr<RouterImpl> pImpl;
};
}
#endif
