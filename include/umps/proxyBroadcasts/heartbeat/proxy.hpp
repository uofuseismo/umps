#ifndef UMPS_PROXYBROADCASTS_HEARTBEAT_BROADCAST_HPP
#define UMPS_PROXYBROADCASTS_HEARTBEAT_BROADCAST_HPP
#include <memory>
#include "umps/proxyBroadcasts/proxy.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
 namespace Logging
 {
  class ILog;
 }
 namespace Services::ConnectionInformation
 {
  class Details;
 }
 namespace ProxyBroadcasts::Heartbeat
 {
  class ProxyOptions;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::ProxyBroadcasts::Heartbeat
{
/// @class Proxy "proxy.hpp" "umps/proxyBroadcasts/heartbeat/proxy.hpp"
/// @brief This defines the XPUB/XSUB proxy to broadcast heartbeats.
///        This is an intermediate message layer to which producers
///        can publish messages to this XSUB socket and from which
///        subscribers can receive content from this XPUB socket.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Proxy : public IProxy
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructs the XPUB/XSUB proxy.
    Proxy();
    /// @brief Constructs the proxy with a given logger.
    explicit Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs the given authenticator. 
    explicit Proxy(std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @brief Constructs the proxy with a given logger and authenticator.
    Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
          std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @}

    /// @name Initialization
    /// @{

    /// @brief Initializes the proxy.
    /// @param[in] options  The proxy options.   
    /// @throws std::invalid_argument if the requisite parameters in the options
    ///         are not set.
    void initialize(const ProxyOptions &options);
    /// @result True indicates that the broadcast is initialized.
    [[nodiscard]] bool isInitialized() const noexcept override final;
    /// @result The name of the broadcast.
    [[nodiscard]] std::string getName() const override final;
    /// @result The connection details.
    [[nodiscard]] Services::ConnectionInformation::Details
        getConnectionDetails() const override final;
    /// @}

    /// @name Proxy Management
    /// @{

    /// @brief Starts the proxy and authenticator service.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start() override final;
    /// @result True indicates the proxy and authenticator are running.
    bool isRunning() const noexcept;
    /// @brief Stops the proxy and authenticator.
    void stop() override final;
    /// @}

    /// @result An uninitialized instance of this class.
    //[[nodiscard]] std::unique_ptr<IProxy> createInstance() const noexcept override final;

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Proxy() override;
    /// @}

    Proxy(const Proxy &proxy) = delete;
    Proxy(Proxy &&proxy) noexcept = delete;
    Proxy& operator=(const Proxy &proxy) = delete;
    Proxy& operator=(Proxy &&proxy) noexcept = delete;
private:
    class ProxyImpl;
    std::unique_ptr<ProxyImpl> pImpl;
};
};
#endif
