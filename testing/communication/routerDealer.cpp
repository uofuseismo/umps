#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include "umps/logging/standardOut.hpp"
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/text.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace UMPS::Messaging::RouterDealer;
namespace UAuth = UMPS::Authentication;

// Faces internal network (sub)
const std::string frontendAddress = "tcp://127.0.0.1:5555";
// Faces external network (pub)
const std::string backendAddress = "tcp://127.0.0.1:5556";
const int nMessages = 5;
const int nThreads = 2;


class ProcessData
{
public:
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        process(const std::string &messageType,
                const void *messageContents, const size_t length)
    {   
        UMPS::MessageFormats::Text request;
        auto response
            = std::make_unique<UMPS::MessageFormats::Text> ();
        //std::cout << "Checking: " << messageType << " " << request.getMessageType() << std::endl;
        if (messageType == request.getMessageType())
        {
            request.fromMessage(
                reinterpret_cast<const char *> (messageContents), length);
            response->setContents(request.getContents() + " handled");
            nResponses = nResponses + 1;
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

void proxy()
{
    ProxyOptions options;
    UAuth::ZAPOptions zapOptions;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(100);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(200);
    //options.setTopic(topic);
    options.setZAPOptions(zapOptions);
    // Make a logger
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    // Initialize the server
    Proxy proxy(loggerPtr);
    EXPECT_NO_THROW(proxy.initialize(options));
    // A thread runs the proxy
    std::thread t1(&Proxy::start, &proxy);
    // Main thread waits...
    std::this_thread::sleep_for(std::chrono::seconds(3));
    /// Main thread tells proxy to stop
    EXPECT_NO_THROW(proxy.stop());
    t1.join();
}

void client(int id) 
{
    UMPS::Logging::StandardOut logger;
    UMPS::MessageFormats::Text text;
    logger.setLevel(UMPS::Logging::Level::Info);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    RequestOptions options;
    options.setAddress(frontendAddress); 
    auto messageType = text.createInstance();
    UMPS::MessageFormats::Messages messageFormats;
    messageFormats.add(messageType);

    options.setMessageFormats(messageFormats);
    Request client;
    EXPECT_NO_THROW(client.initialize(options));
    EXPECT_TRUE(client.isInitialized());
    // Deal with the slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    for (int i = 0; i < nMessages; ++i)
    {
        // Define message to send
        auto messageContents = std::to_string(i) + " ";
        auto responseMessageContents = messageContents + " handled";
        text.setContents(messageContents);
        auto message = client.request(text);
        auto response
            = static_unique_pointer_cast<UMPS::MessageFormats::Text>
              (std::move(message));
        EXPECT_EQ(response->getContents(), responseMessageContents);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }   
}

void server()
{
    // Make a logger
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO); 
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    ProcessData pStruct;
    // Initialize the server
    ReplyOptions options;
    options.setAddress(backendAddress);
    options.setCallback(std::bind(&ProcessData::process,
                        &pStruct, //process,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3));

    Reply server(loggerPtr);
    server.initialize(options);

    // Launch the server
    std::thread t1(&Reply::start, &server);

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

TEST(Messaging, RouterDealer)
{
    // Create the proxy then wait before connecting servers/clients
    auto proxyThread = std::thread(proxy);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Create the tool that can process messages before the messages
    // are created
    auto serverThread = std::thread(server);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Now create the requestors
    auto clientThread1 = std::thread(client, 1);
    auto clientThread2 = std::thread(client, 2);

    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    proxyThread.join();
    serverThread.join();
    clientThread1.join();
    clientThread2.join();
}

}
