#ifndef UMPS_PROXYSERVICES_PROXY_HPP
#define UMPS_PROXYSERVICES_PROXY_HPP
#include <memory>
#include <string>
#include "umps/services/connectionInformation/details.hpp"
namespace UMPS::ProxyServices
{
/// @class IProxy "proxy.hpp" "umps/proxyServices/proxy.hpp"
/// @brief This is the abstract base class which defines the proxy defining
///        an router/dealer-based request/reply mechanism.  The router/dealer
///        is a lightweight intermediary that forwards requests from clients
///        to servers whereby the servers create a response for the request.
///        This response is then propagated back to the client via the
///        router/dealer proxy.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IProxy
{
public:
    /// @brief Destructor.
    virtual ~IProxy() = default;
    /// @result Returns an instance of this broadcast.
    //[[nodiscard]] virtual std::unique_ptr<IProxy> createInstance() const noexcept = 0;
    /// @result True indicates that the publisher is initialized.
    [[nodiscard]] virtual bool isInitialized() const noexcept = 0;
    /// @result The name of the publisher.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] virtual std::string getName() const = 0;
    /// @result The connection details for connecting to the broadcast.
    [[nodiscard]] virtual Services::ConnectionInformation::Details getConnectionDetails() const = 0;
    /// @brief Starts the broadcast.
    /// @note This will create background threads that manage the underlying
    ///       proxy and authenticator service.
    virtual void start() = 0;
    /// @brief Stops the broadcast.
    /// @note This will join the threads that manage the underlying proxy
    ///       proxy and authenticator service. 
    virtual void stop() = 0;
};
}
#endif
