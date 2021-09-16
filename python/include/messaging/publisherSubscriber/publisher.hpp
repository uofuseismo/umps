#ifndef PYURTS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#define PYURTS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#include <memory>
#include <pybind11/pybind11.h>
namespace URTS::Messaging::PublisherSubscriber
{
class Publisher;
}
namespace PURTS::Messaging::PublisherSubscriber
{
class Publisher
{
public:
    Publisher();
    ~Publisher();

    Publisher(const Publisher &publisher) = delete;
    Publisher& operator=(const Publisher &publisher) = delete;
private:
    std::unique_ptr<URTS::Messaging::PublisherSubscriber::Publisher> pImpl; 
};
void initializePublisher(pybind11::module &m);
}
#endif
