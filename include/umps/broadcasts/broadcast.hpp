#ifndef UMPS_BROADCASTS_BROADCAST_HPP
#define UMPS_BROADCASTS_BROADCAST_HPP
#include <memory>
#include <string>
#include "umps/services/connectionInformation/details.hpp"
namespace UMPS::Broadcasts
{
/// @class IBroadcast "broadcast.hpp" "umps/broadcasts/broadcast.hpp"
/// @brief This is the abstract base class which defines a broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IBroadcast
{
public:
    /// @result True indicates that the publisher is initialized.
    [[nodiscard]] virtual bool isInitialized() const noexcept = 0;
    /// @result The name of the publisher.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] virtual std::string getName() const = 0;
    /// @result The address on which a publisher would feed the broadcast. 
    /// @throws std::runtime_error if \c isInitialized() is false.
//    [[nodiscard]] virtual std::string getPublisherAddress() const = 0;
    /// @result The address on which a subscriber would listen to the broadcast.
    /// @throws std::runtime_error if \c isInitialized() is false.
//    [[nodiscard]] virtual std::string getSubscriberAddress() const = 0;
    /// @result The connection details for connecting to the broadcast.
    [[nodiscard]] virtual Services::ConnectionInformation::Details getConnectionDetails() const = 0;
    /// @brief Starts the broadcast.
    /// @note This would be run like:
    ///       Broadcast thisBroadcast;
    ///       std::thread broadcastThread(&Broadcast::start, &thisBroadcast);
    ///       .
    ///       .
    ///       .
    ///       thisBroadcast.stop(); // Called by main thread
    ///       broadcastThread.join();
    virtual void start() = 0;
    /// @brief Stops the broadcast.
    virtual void stop() = 0;
};
}
#endif
