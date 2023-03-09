#include <string>
#include <thread>
#include <umps/messaging/xPublisherXSubscriber/publisher.hpp>
#include <umps/messaging/xPublisherXSubscriber/publisherOptions.hpp>
#include <umps/messageFormats/text.hpp>
#include "xpubxsub.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;

void publisher()
{
    PublisherOptions publisherOptions;
    publisherOptions.setAddress("tcp://127.0.0.1:5555"); // Connect to this address

    Publisher publisher;
    publisher.initialize(publisherOptions);
    // Deal with slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Send messages
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
