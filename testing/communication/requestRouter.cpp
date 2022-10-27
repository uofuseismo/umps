#include <iostream>
#include <functional>
#include <string>
#include <thread>
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/proxyServices/incrementer/incrementRequest.hpp"
#include "umps/proxyServices/incrementer/incrementResponse.hpp"
#include "umps/logging/standardOut.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

const std::string serverHost = "tcp://*:5555"; 
const std::string localHost  = "tcp://127.0.0.1:5555";
int nMessages = 10;
int nThreads = 2;

class ProcessData
{
public:
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        process(const std::string &messageType,
                const void *messageContents, const size_t length)
    {
        UMPS::ProxyServices::Incrementer::IncrementRequest request;
        auto response
            = std::make_unique<UMPS::ProxyServices::Incrementer::IncrementResponse> ();
        //std::cout << "Checking: " << messageType << " " << request.getMessageType() << std::endl;
        if (messageType == request.getMessageType())
        {
            request.fromMessage(
                reinterpret_cast<const char *> (messageContents), length);
            response->setIdentifier(request.getIdentifier());
            //std::cout << "Unpacking: " << request.getIdentifier() << std::endl;
            response->setValue(request.getIdentifier());
            response->setReturnCode(
                UMPS::ProxyServices::Incrementer::ReturnCode::SUCCESS);
            nResponses = nResponses + 1;
        }
        else
        {
            response->setReturnCode(
                UMPS::ProxyServices::Incrementer::ReturnCode::NO_ITEM);
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
std::unique_ptr<UMPS::MessageFormats::IMessage>
    process(const std::string &messageType,
            const uint8_t *messageContents, const size_t length)
{
    UMPS::Services::Incrementer::Request request;
    auto response = std::make_unique<UMPS::ProxyServices::Incrementer::Response> ();
    std::cout << "Checking: " << messageType << " " << request.getMessageType() << std::endl;
    if (messageType == request.getMessageType())
    {
        request.fromCBOR(messageContents, length);
        response->setIdentifier(request.getIdentifier());
        std::cout << "Unpacking: " << request.getIdentifier() << std::endl;
        response->setValue(request.getIdentifier());
        response->setReturnCode(
            UMPS::ProxyServices::Incrementer::ReturnCode::SUCCESS);
    }
    else
    {
        response->setReturnCode(
            UMPS::ProxyServices::Incrementer::ReturnCode::NO_ITEM);
    }
    return response; 
}
*/

void server()
{
    // Make a logger
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO); 
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    ProcessData pStruct;
    // Initialize the server
    UMPS::Messaging::RequestRouter::RouterOptions routerOptions;
    routerOptions.setAddress(serverHost);
    routerOptions.setCallback(std::bind(&ProcessData::process,
                              &pStruct, //process,
                              std::placeholders::_1,
                              std::placeholders::_2,
                              std::placeholders::_3));
    std::unique_ptr<UMPS::MessageFormats::IMessage> messageSubscriptionType
        = std::make_unique<UMPS::ProxyServices::Incrementer::IncrementRequest> ();
    //routerOptions.addMessageFormat(messageSubscriptionType);

    UMPS::Messaging::RequestRouter::Router server(loggerPtr);
    server.initialize(routerOptions);
/*
    server.bind(serverHost);
    server.setCallback(std::bind(&ProcessData::process,
                                 &pStruct, //process,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3));
    std::unique_ptr<UMPS::MessageFormats::IMessage> messageSubscriptionType
        = std::make_unique<UMPS::ProxyServices::Incrementer::Request> ();
    server.addMessageType(messageSubscriptionType);
*/
    // Launch the server
    std::thread t1(&UMPS::Messaging::RequestRouter::Router::start,
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
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);

    UMPS::Messaging::RequestRouter::RequestOptions requestOptions;
    UMPS::ProxyServices::Incrementer::IncrementRequest request;
    std::unique_ptr<UMPS::MessageFormats::IMessage> responseType
        = std::make_unique<UMPS::ProxyServices::Incrementer::IncrementResponse> (); 
    requestOptions.setAddress(localHost);
    requestOptions.addMessageFormat(responseType);
/*
    client.setResponse(responseType);
*/

 
    UMPS::Messaging::RequestRouter::Request client(loggerPtr);
    client.initialize(requestOptions);
    
    request.setItem("Test");
    //client.connect(localHost);
    EXPECT_TRUE(client.isInitialized());
    for (int i = 0; i < nMessages; ++i)
    {
        request.setIdentifier(base + i);
        auto message = client.request(request);
        auto response
        = static_unique_pointer_cast<UMPS::ProxyServices::Incrementer::IncrementResponse>
          (std::move(message));
        EXPECT_EQ(request.getIdentifier(), response->getIdentifier());
        //std::cout << request.getIdentifier() << " " << response->getIdentifier() << std::endl;//Identifier() << std::endl;
    }
}

TEST(Messaging, RequestRouter)
{
    //std::shared_ptr<void *> context = std::make_shared<void *> (zmq_ctx_new()); //zmq::context_t context{1};    
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    auto serverThread  = std::thread(server);
    auto clientThread1 = std::thread(client, 100);
    auto clientThread2 = std::thread(client, 200);

    serverThread.join();
    clientThread1.join();
    clientThread2.join();
}

}
