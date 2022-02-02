#ifndef UMPS_PROXYSERVICES_PACKETCACHE_REPLY_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_REPLY_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class Reply;
 }
 namespace ProxyServices::PacketCache
 {
  class ReplyOptions;
  template<class T> class CappedCollection;
 }
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class Reply "reply.hpp" "umps/proxyServices/packetCache/reply.hpp"
/// @brief A ZeroMQ reply for mechanism for the server in the packet cache.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
class Reply
{
public:
    /// @name Constructors
    /// @{

    Reply();
    /// @brief Constructs a request socket with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Reply(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the reply.
    /// @param[in] options   The reply options.
    /// @throws std::invalid_argument if the endpoint is not set.
    void initialize(const ReplyOptions &options,
                    std::shared_ptr<CappedCollection<T>> &cappedCollection);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Reply getSocketDetails() const;
    /// @}

    /// @name Step 2: Start the Reply Service
    /// @{

    /// @brief Starts the reply service.  The service will poll on messages
    ///        from the dealer, process the messages with the provided callback,
    ///        and return the result to the dealer to propagate via the router
    ///        back to the client. 
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This will spin off a thread that runs in the background so it
    ///       is important to \c stop() the service.
    void start();

    /// @result True indicates that the reply service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @}

    /// @name Step 3: Stop the Reply Service
    /// @{

    /// @brief This will stop the reply service.
    void stop();
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Reply();
    /// @}

    Reply(const Reply &reply) = delete;
    Reply(Reply &&reply) noexcept = delete;
    Reply& operator=(const Reply &reply) = delete;
    Reply& operator=(Reply &&reply) noexcept = delete;
private:
    class ReplyImpl;
    std::unique_ptr<ReplyImpl> pImpl;
};
}
#endif
