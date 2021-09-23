#ifndef PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#define PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "messageFormats/message.hpp"
namespace UMPS::Messaging::PublisherSubscriber
{
class Subscriber;
}
namespace PUMPS::Messaging::PublisherSubscriber
{
class Subscriber
{
public:
    Subscriber();
    ~Subscriber();

    void addSubscription(const PUMPS::MessageFormats::IMessage &message);

    Subscriber(const Subscriber &subscriber) = delete;
    Subscriber& operator=(const Subscriber &subscriber) = delete;
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::Subscriber> pImpl; 
};
void initializeSubscriber(pybind11::module &m);
}
#endif
