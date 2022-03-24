#ifndef UMPS_PROXYSERVICES_PACKETCACHE_REPLIER_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_REPLIER_HPP
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
  class ReplierOptions;
  template<class T> class CappedCollection;
 }
}
namespace zmq
{
 class context_t;
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class Replier "replier.hpp" "umps/proxyServices/packetCache/replier.hpp"
/// @brief A replier mechanism for the server in the packet cache.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
template<class T = double>
class Replier
{
public:
    /// @name Constructors
    /// @{

    Replier();
    /// @brief Constructs a request socket with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Replier(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a request socket with a given logger and context.
    Replier(std::shared_ptr<zmq::context_t> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Step 1: Initialization
    /// @{

    /// @brief Initializes the reply.
    /// @param[in] options           The reply options.
    /// @param[in] cappedCollection  A reference to the underlying capped
    ///                              collection for use by the callback.
    /// @throws std::invalid_argument if the endpoint is not set.
    void initialize(const ReplierOptions &options,
                    std::shared_ptr<CappedCollection<T>> &cappedCollection);
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::Reply getSocketDetails() const;
    /// @}

    /// @name Step 2: Start the Replier Service
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

    /// @name Step 3: Stop the Replier Service
    /// @{

    /// @brief This will stop the reply service.
    void stop();
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Replier();
    /// @}

    Replier(const Replier &reply) = delete;
    Replier(Replier &&reply) noexcept = delete;
    Replier& operator=(const Replier &reply) = delete;
    Replier& operator=(Replier &&reply) noexcept = delete;
private:
    class ReplierImpl;
    std::unique_ptr<ReplierImpl> pImpl;
};
}
#endif
