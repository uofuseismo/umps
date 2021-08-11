#ifndef URTS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#define URTS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#include <memory>
// Forward declarations
namespace URTS
{
 namespace Logging
 {
  class ILog;
 }
 namespace MessageFormats
 {
  class IMessage;
 }
}
namespace zmq
{
 class context_t;
}
namespace URTS::Messaging::PublisherSubscriber
{
/// @class Publisher "publisher.hpp" "urts/messaging/publisherSubscriber/publisher.hpp"
/// @brief A ZeroMQ publisher.
/// @detail You can think of a publisher as a radio broadcast.  If you, as 
///         a subscriber, are tuned in then you'll hear the broadcast.  
///         However, if you tune in late or turn off your radio then
///         you'll miss those sections of the broadcast.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Publisher
{
public:
    /// @brief Default constructor.
    Publisher();
    /// @brief Constructs a publisher with the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit Publisher(std::shared_ptr<URTS::Logging::ILog> &logger);
    /// @brief Constructor.
    /// @param[in] context  The context from which to initialize.
    /// @note This can be useful for inproc communication where a separate
    ///       thread IO thread is not required.  In this case, the context
    ///       can be made with:
    ///       auto context = std::shared_ptr<zmq::context_t> (0).
    explicit Publisher(std::shared_ptr<zmq::context_t> &context);
    /// @brief Move constructor.
    /// @param[in,out] publisher  The publisher class from which to initialize
    ///                           this class.  On exit, publisher's behavior is
    ///                           undefined.
    Publisher(Publisher &&publisher) noexcept;

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] publisher  The publisher class whose memory will be
    ///                           moved to this.  On exit, publisher's
    ///                           behavior is undefined.
    /// @result The memory from publisher moved to this.
    Publisher& operator=(Publisher &&publisher) noexcept;
    /// @}

    /// @param[in] endPoint  The endpoint for accepting connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @note For more see: http://api.zeromq.org/2-1:zmq-bind. 
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void bind(const std::string &endPoint);

    /// @name Destructors
    /// @{
    /// @brief Destructor.
    ~Publisher();
    /// @}
 
    void send(const MessageFormats::IMessage &message);
    void send(size_t nBytes, const char *message);
    void send(const std::string &message);

    /// Delete some functions
    Publisher(const Publisher &publisher) = delete;
    Publisher& operator=(const Publisher &publisher) = delete;
private:
    class PublisherImpl;
    std::unique_ptr<PublisherImpl> pImpl;
};
}
#endif
