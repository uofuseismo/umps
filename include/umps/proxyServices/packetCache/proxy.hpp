#ifndef UMPS_PROXYSERVICES_PACKETCACHE_PROXY_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_PROXY_HPP
#include <memory>
#include "umps/proxyServices/proxy.hpp"
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
 namespace ProxyServices::PacketCache
 {
  class ProxyOptions;
 }
 namespace Authentication
 {
  class IAuthenticator;
 }
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class Proxy "proxy.hpp" "umps/proxyServices/packetCache/proxy.hpp"
/// @brief This defines the ROUTER/DEALER proxy to process waveform
///        packets for a channel.  This is an intermediate message layer
///        to which clients send requests to the frontend which is
///        then propagated to the backend and the servers.  The
///        servers then send a reply to the backend which is propagated
///        to the frontend and, ultimately, the original client.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Proxy : public IProxy
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructs the XPUB/XSUB proxy.
    Proxy();
    /// @brief Constructs the XPUB/XSUB with a given logger.
    explicit Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a given logger and authenticator.
    Proxy(std::shared_ptr<UMPS::Logging::ILog> &logger,
          std::shared_ptr<UMPS::Authentication::IAuthenticator> &authenticator);
    /// @}

    /// @brief Initializes the proxy.
    /// @param[in] options  The proxy options.  At a minimum, this must 
    ///                     contain the frontend and backend address.
    /// @throws std::invalid_argument if the options are invalid.
    void initialize(const ProxyOptions &options);

    /// @name Initialization
    /// @{

    /// @result True indicates that the proxy is initialized.
    [[nodiscard]] bool isInitialized() const noexcept override final;
    /// @result The name of the proxy broadcast.
    [[nodiscard]] std::string getName() const override final;
    /// @result The connection details.
    [[nodiscard]] Services::ConnectionInformation::Details
        getConnectionDetails() const override final;
    /// @}

    /// @name Starts the proxy.
    /// @{
    /// @brief Starts the proxy.
    /// @throws std::runtime_error if \c isInitialized() is false.
    void start() override final;
    /// @result True indicates the proxy is running.
    bool isRunning() const noexcept;
    /// @brief Stops the proxy.
    void stop() override final;
    /// @}

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
}
#endif
