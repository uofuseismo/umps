#ifndef UMPS_PROXYBROADCASTS_HEARTBEAT_PUBLISHERPROCESSOPTIONS
#define UMPS_PROXYBROADCASTS_HEARTBEAT_PUBLISHERPROCESSOPTIONS
#include <memory>
#include <chrono>
namespace UMPS
{
 namespace Services::ConnectionInformation
 {
  class Requestor;
 }
 namespace ProxyBroadcasts::Heartbeat
 {
  class Publisher;
  class PublisherOptions;
 }
}
namespace UMPS::ProxyBroadcasts::Heartbeat
{
/// @class PublisherProcessOptions publisherProcessOptions.hpp "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
/// @brief This process will send status messages to the heartbeat broadcast
///        at regular intervals. 
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class PublisherProcessOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    PublisherProcessOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize this
    ///                     class. 
    PublisherProcessOptions(const PublisherProcessOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    PublisherProcessOptions(PublisherProcessOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options.
    PublisherProcessOptions& operator=(const PublisherProcessOptions &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this. 
    PublisherProcessOptions& operator=(PublisherProcessOptions &&options) noexcept;
    /// @}

    /// @brief Sets the time interval to send messages.
    /// @throws std::invalid_argument if the interval is not positive.
    void setInterval(const std::chrono::seconds &interval);
    /// @reslt The time interval between status messages.
    [[nodiscard]] std::chrono::seconds getInterval() const noexcept;
 
    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~PublisherProcessOptions();
    /// @}
private:
    class PublisherProcessOptionsImpl;
    std::unique_ptr<PublisherProcessOptionsImpl> pImpl;
};
}
#endif
