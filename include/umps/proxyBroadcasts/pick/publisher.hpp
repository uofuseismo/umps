#ifndef UMPS_PROXYBROADCASTS_PICK_PUBLISHER_HPP
#define UMPS_PROXYBROADCASTS_PICK_PUBLISHER_HPP
#include <memory>
#include "umps/authentication/enums.hpp"
// Forward declarations
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
 namespace ProxyBroadcasts::Pick
 {
  class PublisherOptions;
 }
 namespace MessageFormats
 {
  class Pick;
 }
 namespace Services::ConnectionInformation::SocketDetails
 {
  class XPublisher;
 }
}
namespace UMPS::ProxyBroadcasts::Pick
{
/// @class Publisher "publisher.hpp" "umps/proxyBroadcasts/pick/publisher.hpp"
/// @brief A ZeroMQ publisher specialized for sending pick messages.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Publisher
{
public:
    /// @name Constructors
    /// @{

    /// @brief Default constructor.
    Publisher();
    /// @brief Constructs a publisher with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructs a publisher with a given ZeroMQ context.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<UMPS::Messaging::Context> (0).
    explicit Publisher(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Construtcs a publisher with a given ZeroMQ context and logger.
    Publisher(std::shared_ptr<UMPS::Messaging::Context> &context,
              std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] publisher  The publisher class from which to initialize
    ///                           this class.  On exit, publisher's behavior is
    ///                           undefined.
    Publisher(Publisher &&publisher) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Move assignment operator.
    /// @param[in,out] publisher  The publisher class whose memory will be
    ///                           moved to this.  On exit, publisher's
    ///                           behavior is undefined.
    /// @result The memory from publisher moved to this.
    Publisher& operator=(Publisher &&publisher) noexcept;
    /// @}

    /// @brief Initializes the publisher.
    /// @param[in] options  The publisher options.  This must have an address.
    /// @throws std::invalid_argument if \c options.haveAddress().
    void initialize(const PublisherOptions &options);
    /// @result True indicates that the subscriber is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result The details for connecting to this socket.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] Services::ConnectionInformation::SocketDetails::XPublisher getSocketDetails() const;

    /// @brief Sends a message.  This will serialize the message.
    /// @param[in] message  The message to send.
    /// @throws std::runtime_error if the class is not initialized.
    /// @throws std::invalid_argument if the message cannot be serialized.
    void send(const MessageFormats::Pick &message);

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Publisher();
    /// @}

    /// Delete some functions
    Publisher(const Publisher &publisher) = delete;
    Publisher& operator=(const Publisher &publisher) = delete;
private:
    class PublisherImpl;
    std::unique_ptr<PublisherImpl> pImpl;
};
}
#endif