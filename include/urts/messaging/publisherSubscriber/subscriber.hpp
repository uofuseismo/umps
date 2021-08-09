#ifndef URTS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#define URTS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
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
class Subscriber
{
public:
    /// @brief Default constructor.
    Subscriber();
    /// @brief Constructor with specified logger.
    explicit Subscriber(std::shared_ptr<URTS::Logging::ILog> &logger);
    explicit Subscriber(std::shared_ptr<zmq::context_t> &context);
    //explicit Subscriber(std::shared_ptr<void *> &context);
    /// @brief Move constructor.
    /// @param[in,out] subscriber  The subscriber class from which to initialize
    ///                            this class.  On exit, subscriber's behavior
    ///                            is undefined.
    Subscriber(Subscriber &&subscriber) noexcept;

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] subscriber  The subscriber class whose memory will be
    ///                            moved to this.  On exit, the subscriber's
    ///                            behavior is undefined.
    /// @result The memory from subscriber moved to this.
    Subscriber& operator=(Subscriber &&subscriber) noexcept;
    /// @}

    /// @name Step 1: Connect to the Subscriber
    /// @{
    /// @brief Connects to the endpoint.
    /// @param[in] endPoint  The endpoint to which to connect.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @note For more see: http://api.zeromq.org/3-2:zmq-connect
    /// @throws std::runtime_error if zeromq failed to bind to given endpoint.
    void connect(const std::string &endPoint);
    /// @result True indicates that the class is connected.
    [[nodiscard]] bool isConnected() const noexcept;

    /// @name Step 2: Add Subscription
    /// @{
    /// @param[in] message  The message format to which to subscribe.
    ///                     The message type is given by
    ///                     message.getMessageType().
    /// @throws std::invalid_argument if message is NULL.
    /// @throws std::runtime_error if \c isConnected() is false. 
    void addSubscription(std::unique_ptr<URTS::MessageFormats::IMessage> &message);
    /// @result True indicates that there are subscriptions. 
    [[nodiscard]] bool haveSubscriptions() const noexcept;
    /// @}
  
    /// @name Destructors
    /// @{
    /// @brief Destructor.
    ~Subscriber();
    /// @}
 
    /// @brief Receives a message.
    /// @throws std::invalid_argument if the message cannot be serialized.
    std::unique_ptr<MessageFormats::IMessage> receive() const;
    //void send(size_t nBytes, const char *message);
    //void send(const std::string &message);

    /// Delete some functions
    Subscriber(const Subscriber &subscriber) = delete;
    Subscriber& operator=(const Subscriber &subscriber) = delete;
private:
    class SubscriberImpl;
    std::unique_ptr<SubscriberImpl> pImpl;
};
}
#endif
