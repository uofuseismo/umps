#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/pick.hpp"
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

const std::string network = "UU";
const std::string station = "NOQ";
const std::string channel = "EHZ";
const std::string locationCode = "01";
const std::string phaseHint = "P";
const std::string algorithm = "superPicker";
const std::string newAlgorithm = algorithm + "_extra";
const double time = 5600;
const uint64_t idBase = 100;
const UMPS::MessageFormats::Pick::Polarity polarity = UMPS::MessageFormats::Pick::Polarity::DOWN;
const int nMessages = 5;
const int nThreads = 2;


class ProcessData
{
public:
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        process(const std::string &messageType,
                const void *messageContents, const size_t length)
    {   
        UMPS::MessageFormats::Pick request;
        auto response
            = std::make_unique<UMPS::MessageFormats::Pick> ();
        //std::cout << "Checking: " << messageType << " " << request.getMessageType() << std::endl;
        if (messageType == request.getMessageType())
        {
            request.fromMessage(
                reinterpret_cast<const char *> (messageContents), length);
            response->setIdentifier(request.getIdentifier());
            //std::cout << "Unpacking: " << request.getIdentifier() << std::endl;
            response->setNetwork(request.getNetwork());
            response->setStation(request.getStation());
            response->setChannel(request.getChannel());
            response->setLocationCode(request.getLocationCode());
            response->setPhaseHint(request.getPhaseHint());
            response->setTime(request.getTime());
            response->setPolarity(request.getPolarity());
            response->setAlgorithm(newAlgorithm); // Change the algorithm
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
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Initialize the server
    Proxy proxy(loggerPtr);
    proxy.initialize(options);
    // A thread runs the proxy
    std::thread t1(&Proxy::start, &proxy);
    // Main thread waits...
    std::this_thread::sleep_for(std::chrono::seconds(3));
    /// Main thread tells proxy to stop
    proxy.stop();
    t1.join();
}

void client(int id) 
{
    UMPS::Logging::StdOut logger;
    UMPS::MessageFormats::Pick pick;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    RequestOptions options;
    options.setEndPoint(frontendAddress); 
    auto pickType = pick.createInstance();
    UMPS::MessageFormats::Messages messageFormats;
    messageFormats.add(pickType);

    options.setMessageFormats(messageFormats);
    Request client;
    EXPECT_NO_THROW(client.initialize(options));
    EXPECT_TRUE(client.isInitialized());
    // Deal with the slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Define message to send
    pick.setTime(time);
    pick.setNetwork(network);
    pick.setStation(station);
    pick.setChannel(channel);
    pick.setLocationCode(locationCode);
    pick.setPhaseHint(phaseHint);
    pick.setPolarity(polarity);
    pick.setAlgorithm(algorithm);

    for (int i = 0; i < nMessages; ++i)
    {   
        pick.setIdentifier(idBase + i); 
        auto message = client.request(pick);
        auto response
            = static_unique_pointer_cast<UMPS::MessageFormats::Pick>
              (std::move(message));
        EXPECT_EQ(pick.getNetwork(),      response->getNetwork());
        EXPECT_EQ(pick.getStation(),      response->getStation());
        EXPECT_EQ(pick.getChannel(),      response->getChannel());
        EXPECT_EQ(pick.getLocationCode(), response->getLocationCode());
        EXPECT_EQ(pick.getPhaseHint(),    response->getPhaseHint());
        EXPECT_EQ(pick.getPolarity(),     response->getPolarity());
        EXPECT_EQ(newAlgorithm,           response->getAlgorithm());
        EXPECT_NEAR(std::abs(pick.getTime() - response->getTime()), 0, 1.e-14);
        EXPECT_EQ(pick.getIdentifier(),   response->getIdentifier());

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }   
}

void server()
{
    // Make a logger
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO); 
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    ProcessData pStruct;
    // Initialize the server
    ReplyOptions options;
    options.setEndPoint(backendAddress);
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
