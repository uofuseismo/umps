#include <iostream>
#include <string>
#include <umps/messaging/publisherSubscriber/subscriber.hpp>
#include <umps/messaging/publisherSubscriber/subscriberOptions.hpp>
#include <umps/messageFormats/text.hpp>
#include <umps/messageFormats/messages.hpp>
#include <umps/messageFormats/staticUniquePointerCast.hpp>
#include "pubsub.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;

void subscriber(const int subscriberID)
{
    // Define the message types that the subscriber will receive
    UMPS::MessageFormats::Messages messageTypes;
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessageType
        = std::make_unique<UMPS::MessageFormats::Text> (); 
    messageTypes.add(textMessageType);

    // Define the subscriber options
    SubscriberOptions subscriberOptions;
    subscriberOptions.setAddress("tcp://127.0.0.1:5555"); // Connect to this address
    subscriberOptions.setMessageTypes(messageTypes); // Types of messages to get

    // Initialize the subscriber
    Subscriber subscriber;
    subscriber.initialize(subscriberOptions);

    // Now retrieve messages
    for (int i = 0; i < N_MESSAGES; ++i)
    {
        // The message is read off the wire by ZeroMQ and deserialized
        // by the Text class.
        auto message = subscriber.receive(); // Blocks until message is received
        // Convert the message so we can look at the contents
        auto textMessage
            = static_unique_pointer_cast<UMPS::MessageFormats::Text>
              (std::move(message));
        std::cout << "SubscriberID: " << subscriberID
                  << " received: " << textMessage->getContents() << std::endl;
    }
}
