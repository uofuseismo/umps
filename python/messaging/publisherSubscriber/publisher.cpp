#include "messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "initialize.hpp"

using namespace PUMPS::Messaging::PublisherSubscriber;

Publisher::Publisher() :
    pImpl(std::make_unique<UMPS::Messaging::PublisherSubscriber::Publisher> ()) 
{
}

Publisher::~Publisher() = default;

void PUMPS::Messaging::PublisherSubscriber::initializePublisher(pybind11::module &m) 
{
    pybind11::class_<PUMPS::Messaging::PublisherSubscriber::Publisher>
        o(m, "Publisher");
}
