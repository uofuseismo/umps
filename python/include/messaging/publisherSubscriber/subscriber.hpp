#ifndef PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#define PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#include <memory>
#include <vector>
#include <pybind11/pybind11.h>
#include "messageFormats/message.hpp"
#include "umps/authentication/enums.hpp"
namespace UMPS
{
 namespace MessageFormats
 {
  class Messages;
 }
 namespace Messaging::PublisherSubscriber
 {
  class Subscriber;
 }
}
namespace PUMPS
{
 namespace MessageFormats
 {
  class IMessage;
  class Messages;
 }
 namespace Messaging::PublisherSubscriber
 {
  class SubscriberOptions;
 }
}
namespace PUMPS::Messaging::PublisherSubscriber
{
class Subscriber
{
public:
    Subscriber();
    ~Subscriber();

    void initialize(const SubscriberOptions &options);
    [[nodiscard]] bool isInitialized() const noexcept;
    //void addSubscription(const PUMPS::MessageFormats::IMessage &message);
    [[nodiscard]] std::string getEndPoint() const;
    [[nodiscard]] UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept; 
    [[nodiscard]] std::unique_ptr<PUMPS::MessageFormats::IMessage> receive() const;
    void disconnect();

    Subscriber(const Subscriber &subscriber) = delete;
    Subscriber& operator=(const Subscriber &subscriber) = delete;
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::Subscriber> mSubscriber;
    std::unique_ptr<UMPS::MessageFormats::Messages> mMessageTypes;
    //std::vector<std::unique_ptr<PUMPS::MessageFormats::IMessage>> mPythonMessageTypes;
};
}
#endif
