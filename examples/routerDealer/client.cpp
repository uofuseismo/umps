#include <iostream>
#include <string>
#include <umps/messageFormats/failure.hpp>
#include <umps/messageFormats/messages.hpp>
#include <umps/messaging/routerDealer/request.hpp>
#include <umps/messaging/routerDealer/requestOptions.hpp>
#include <umps/messageFormats/staticUniquePointerCast.hpp>
#include "routerDealer.hpp"
#include "messageTypes.hpp"

void client(int instance)
{
    // Define the message types that the client will receive
    UMPS::MessageFormats::Messages messageTypes;
    std::unique_ptr<UMPS::MessageFormats::IMessage> replyMessageType
        = std::make_unique<::ReplyMessage> ();
    std::unique_ptr<UMPS::MessageFormats::IMessage> failureMessageType
        = std::make_unique<UMPS::MessageFormats::Failure> (); 
    messageTypes.add(replyMessageType);
    messageTypes.add(failureMessageType);

    // Define the subscriber options
    UMPS::Messaging::RouterDealer::RequestOptions clientOptions;
    clientOptions.setAddress("tcp://127.0.0.1:5555"); // Connect to this address
    clientOptions.setReceiveTimeOut(std::chrono::milliseconds {500});
    clientOptions.setMessageFormats(messageTypes);

    // Initialize the subscriber
    UMPS::Messaging::RouterDealer::Request client;
    client.initialize(clientOptions);

    // Process some requests
    for (int i = 0; i < N_REQUESTS; ++i)
    {
        ::RequestMessage request;
        request.setContents("Request from instance "
                          + std::to_string(instance));
        auto reply = client.request(request);
        if (reply != nullptr)
        {
            ::ReplyMessage replyMessage;
            if (reply->getMessageType() == replyMessage.getMessageType())
            {
                auto replyMessage
                     = UMPS::MessageFormats::static_unique_pointer_cast
                             <::ReplyMessage> (std::move(reply));
                std::cout << "Reply thread " << instance << " received reply: "
                          << replyMessage->getContents() << std::endl;
            }
            else
            {
                std::cout << "Server reply to client " << instance
                          << " encountered problems." << std::endl;
            }
        }
        else
        {
            std::cout << "Reply to " << instance << " timed out." << std::endl;
        }
    }
}
