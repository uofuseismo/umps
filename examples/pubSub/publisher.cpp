#include <string>
#include <thread>
#include <umps/messaging/publisherSubscriber/publisher.hpp>
#include <umps/messaging/publisherSubscriber/publisherOptions.hpp>
#include <umps/messageFormats/text.hpp>
#include "pubsub.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;

void publisher()
{
    PublisherOptions publisherOptions;
    publisherOptions.setAddress("tcp://127.0.0.1:5555"); // Bind on this address

    Publisher publisher;
    publisher.initialize(publisherOptions);
    // Wait a bit for the subscriber to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(750));
 
    for (int i = 0; i < 10; ++i)
    {
        // Define a message
        UMPS::MessageFormats::Text textMessage;
        textMessage.setContents("Message number " + std::to_string(i + 1));
        // The message is serialized by the Text class and put on the wire
        // by ZeroMQ
        publisher.send(textMessage);
    }
}
