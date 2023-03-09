#include <string>
#include <thread>
#include <umps/messaging/xPublisherXSubscriber/publisher.hpp>
#include <umps/messaging/xPublisherXSubscriber/publisherOptions.hpp>
#include <umps/messageFormats/text.hpp>
#include "xpubxsub.hpp"

using namespace UMPS::Messaging::XPublisherXSubscriber;

void publisher(const int publisherID)
{
    PublisherOptions publisherOptions;
    publisherOptions.setAddress("tcp://127.0.0.1:5555"); // Connect to this address

    Publisher publisher;
    publisher.initialize(publisherOptions);
    // Deal with slow joiner problem.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Send messages
    for (int i = 0; i < N_MESSAGES; ++i)
    {
        UMPS::MessageFormats::Text textMessage;
        textMessage.setContents("Message number "
                              + std::to_string(i + 1)
                              + " from publisher "
                              + std::to_string(publisherID));
        publisher.send(textMessage);
    }
}
