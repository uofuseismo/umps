#ifndef PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#define PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHER_HPP
#include <memory>
#include <pybind11/pybind11.h>
namespace UMPS::Messaging::PublisherSubscriber
{
class Publisher;
}
namespace PUMPS::Messaging::PublisherSubscriber
{
class Publisher
{
public:
    Publisher();
    ~Publisher();

    Publisher(const Publisher &publisher) = delete;
    Publisher& operator=(const Publisher &publisher) = delete;
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::Publisher> pImpl; 
};
void initializePublisher(pybind11::module &m);
}
#endif
