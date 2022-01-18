#ifndef UMPS_BROADCASTS_BROADCAST_HPP
#define UMPS_BROADCASTS_BROADCAST_HPP
#include <memory>
#include <string>
#include "umps/services/connectionInformation/details.hpp"
namespace UMPS::Broadcasts
{
/// @class IBroadcast "broadcast.hpp" "umps/broadcasts/broadcast.hpp"
/// @brief This is the abstract base class which defines a broadcast.  
///        Technically, the broadcast is a lightweight intermediary that
///        forwards messages from producers to a location that is accessible
///        by clients.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IBroadcast
{
public:
    /// @brief Destructor.
    virtual ~IBroadcast() = default;
    /// @result Returns an instance of this broadcast.
    [[nodiscard]] virtual std::unique_ptr<IBroadcast> createInstance() const noexcept = 0;
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
