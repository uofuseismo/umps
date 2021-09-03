#include <iostream>
#include <functional>
#include <string>
#include <thread>
#include "urts/messaging/requestRouter/router.hpp"
#include "urts/messaging/requestRouter/request.hpp"
#include "urts/modules/incrementer/request.hpp"
#include "urts/modules/incrementer/response.hpp"
#include "urts/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

const std::string serverHost = "tcp://*:5555"; 
const std::string localHost  = "tcp://127.0.0.1:5555";
int nMessages = 10;
int nThreads = 2;

/*
std::unique_ptr<URTS::MessageFormats::IMessage> 
    process(const URTS::MessageFormats::IMessage *requestIn)
{
    auto request
        = reinterpret_cast<const URTS::Modules::Incrementer::Request *> (requestIn); 
    auto response = std::make_unique<URTS::Modules::Incrementer::Response> ();
    response->setIdentifier(request->getIdentifier());
    return response;
}
*/

class ProcessData
{
public:
    std::unique_ptr<URTS::MessageFormats::IMessage>
        process(const std::string &messageType,
                const uint8_t *messageContents, const size_t length)
    {
        URTS::Modules::Incrementer::Request request;
        auto response
            = std::make_unique<URTS::Modules::Incrementer::Response> ();
        //std::cout << "Checking: " << messageType << " " << request.getMessageType() << std::endl;
        if (messageType == request.getMessageType())
        {
            request.fromCBOR(messageContents, length);
            response->setIdentifier(request.getIdentifier());
            //std::cout << "Unpacking: " << request.getIdentifier() << std::endl;
            response->setValue(request.getIdentifier());
            response->setReturnCode(
                URTS::Modules::Incrementer::ReturnCode::SUCCESS);
            nResponses = nResponses + 1;
        }
        else
        {
            response->setReturnCode(
                URTS::Modules::Incrementer::ReturnCode::NO_ITEM);
        }
        return response;
    } 
    int getNumberOfResponses() const
    {
        return nResponses;
    }
private:
    int nResponses = 0;
};

/*
std::unique_ptr<URTS::MessageFormats::IMessage>
    process(const std::string &messageType,
            const uint8_t *messageContents, const size_t length)
{
    URTS::Modules::Incrementer::Request request;
    auto response = std::make_unique<URTS::Modules::Incrementer::Response> ();
    std::cout << "Checking: " << messageType << " " << request.getMessageType() << std::endl;
    if (messageType == request.getMessageType())
    {
        request.fromCBOR(messageContents, length);
        response->setIdentifier(request.getIdentifier());
        std::cout << "Unpacking: " << request.getIdentifier() << std::endl;
        response->setValue(request.getIdentifier());
        response->setReturnCode(
            URTS::Modules::Incrementer::ReturnCode::SUCCESS);
    }
    else
    {
        response->setReturnCode(
            URTS::Modules::Incrementer::ReturnCode::NO_ITEM);
    }
    return response; 
}
*/

void server()
{
    // Make a logger
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::INFO); 
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);
    ProcessData pStruct;
    // Initialize the server
    URTS::Messaging::RequestRouter::Router server(loggerPtr);
    server.bind(serverHost);
    server.setCallback(std::bind(&ProcessData::process,
                                 &pStruct, //process,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3));
    //std::unique_ptr<URTS::MessageFormats::IMessage> messageSubscriptionType
    //    = std::make_unique<URTS::Modules::Incrementer::Request> ();
    //server.addSubscription(messageSubscriptionType);
    // Launch the server
    std::thread t1(&URTS::Messaging::RequestRouter::Router::start,
                   &server);
    // Have the main thread kill the server
    while (pStruct.getNumberOfResponses() < nMessages*nThreads)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Wait for other thread to get all messages before closing queue.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Now kill the server
    server.stop();
    t1.join();
}

void client(int base)
{
    // Make a logger
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::INFO);
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);

    URTS::Messaging::RequestRouter::Request client(loggerPtr);
    URTS::Modules::Incrementer::Request request;
    std::unique_ptr<URTS::MessageFormats::IMessage> responseType
        = std::make_unique<URTS::Modules::Incrementer::Response> (); 
    client.setResponse(responseType);

    
    request.setItem("Test");
    client.connect(localHost);
    EXPECT_TRUE(client.isConnected());
    for (int i = 0; i < nMessages; ++i)
    {
        request.setIdentifier(base + i);
        auto message = client.request(request);
        auto response
        = static_unique_pointer_cast<URTS::Modules::Incrementer::Response>
          (std::move(message));
        EXPECT_EQ(request.getIdentifier(), response->getIdentifier());
        //std::cout << request.getIdentifier() << " " << response->getIdentifier() << std::endl;//Identifier() << std::endl;
    }
}

TEST(Messaging, RequestRouter)
{
    //std::shared_ptr<void *> context = std::make_shared<void *> (zmq_ctx_new()); //zmq::context_t context{1};    
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::DEBUG);
    auto serverThread  = std::thread(server);
    auto clientThread1 = std::thread(client, 100);
    auto clientThread2 = std::thread(client, 200);

    serverThread.join();
    clientThread1.join();
    clientThread2.join();
}

}
