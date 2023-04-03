#include <iostream>
#include <thread>
#include <umps/messageFormats/failure.hpp>
#include <umps/messaging/routerDealer/reply.hpp>
#include <umps/messaging/routerDealer/replyOptions.hpp>
#include "routerDealer.hpp"
#include "messageTypes.hpp"

namespace
{

/// This class is run in a thread and responds to client requests.
class MessageProcessor
{
public:
    /// Constructor
    explicit MessageProcessor(int identifier) :
        mIdentifier(identifier)
    {
    }
    /// This function responds to requests from the clients.
    [[nodiscard]]
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        process(const std::string &messageType,
                const void *messageContents, const size_t length)
    {
        UMPS::MessageFormats::Failure failureMessage; 
        // Handle a request message
        ::RequestMessage request;
        if (messageType == request.getMessageType())
        {
            ::ReplyMessage response;
            // Deserialize the message
            try
            {
                request.fromMessage(
                    reinterpret_cast<const char *> (messageContents), length);
            }
            catch (const std::exception &e)
            {
                failureMessage.setDetails("Server "
                                        + std::to_string(mIdentifier)
                                        + " failed to deserialize message");
                return failureMessage.clone();
            }
            // Process the message and set the result
            mResponses = mResponses + 1
            response.setContents("Server " + std::to_string(mIdentifier)
                               + " handling message: " + request.getContents());
            // Send the reply back 
            return response.clone();
        }
        // It's unclear what type of request was received.  Send a generic
        // failure message back to the client.
        failureMessage.setDetails("Server " + std::to_string(mIdentifier)
                                + " encountered unhandled message type "
                                + messageType); 
        return failureMessage.clone();
    }
    [[nodiscard]] int getNumberOfResponses() const
    {
        return mResponses;
    }
private:
    int mIdentifier{0};
    int mResponses{0};
};

}

void server(int instance)
{
    // UMPS will require the backend to have the ability to respond to messages.
    ::MessageProcessor messageProcessor(instance);

    // Create the replier options
    UMPS::Messaging::RouterDealer::ReplyOptions options;
    options.setAddress("tcp://127.0.0.1:5556");
    options.setCallback(std::bind(&MessageProcessor::process,
                                  &messageProcessor,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));

    // Initialize the replier service
    UMPS::Messaging::RouterDealer::Reply server;
    server.initialize(options);

    // Start the server
    auto replierThread
        = std::thread(&UMPS::Messaging::RouterDealer::Reply::start, &server);

    // Main thread sleeps for a bit.
    std::this_thread::sleep_for(std::chrono::seconds {2});

    // Shut down the replier thread.
    server.stop();
    std::cout << "Server instance " << instance << " handled "
              << messageProcessor.getNumberOfResponses()
              << " requests" << std::endl;

    // Join threads
    if (replierThread.joinable()){replierThread.join();}
}
