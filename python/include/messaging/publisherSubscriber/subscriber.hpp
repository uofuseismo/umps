#ifndef PYURTS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#define PYURTS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBER_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "messageFormats/message.hpp"
namespace URTS::Messaging::PublisherSubscriber
{
class Subscriber;
}
namespace PURTS::Messaging::PublisherSubscriber
{
class Subscriber
{
public:
    Subscriber();
    ~Subscriber();

    void addSubscription(const PURTS::MessageFormats::IMessage &message);

    Subscriber(const Subscriber &subscriber) = delete;
    Subscriber& operator=(const Subscriber &subscriber) = delete;
private:
    std::unique_ptr<URTS::Messaging::PublisherSubscriber::Subscriber> pImpl; 
};
void initializeSubscriber(pybind11::module &m);
}
#endif
