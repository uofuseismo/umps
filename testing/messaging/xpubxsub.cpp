#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "urts/logging/stdout.hpp"
#include "urts/messaging/publisherSubscriber/publisher.hpp"
#include "urts/messaging/publisherSubscriber/subscriber.hpp"
#include "urts/messaging/publisherSubscriber/proxy.hpp"
#include "urts/messageFormats/pick.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

//const std::string serverHost = "tcp://*:5555"; 
//const std::string localHost  = "tcp://127.0.0.1:5555";
// Faces internal network (sub)
const std::string frontendAddress = "tcp://127.0.0.1:5555";
// Faces external network (pub)
const std::string backendAddress = "tcp://127.0.0.1:5556";
const std::string topic = "proxyTest";
const std::string network = "UU";
const std::string station = "NOQ";
const std::string channel = "EHZ";
const std::string locationCode = "01";
const std::string phaseHint = "P";
const double time = 5600;
const uint64_t idBase = 100;
const URTS::MessageFormats::Pick::Polarity polarity = URTS::MessageFormats::Pick::Polarity::UP;
using namespace URTS::Messaging::PublisherSubscriber;

void proxy()
{
    // Make a logger
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::INFO);
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);
    // Initialize the server
    URTS::Messaging::PublisherSubscriber::Proxy proxy(loggerPtr);
    proxy.initialize(frontendAddress, backendAddress, topic);
    // A thread runs the proxy
    std::thread t1(&URTS::Messaging::PublisherSubscriber::Proxy::start,
                   &proxy);
    // Main thread waits...
    std::this_thread::sleep_for(std::chrono::seconds(3));
    /// Main thread tells proxy to stop
    proxy.stop();
    t1.join();
}

void publisher(int id)
{
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::INFO);
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);
    URTS::Messaging::PublisherSubscriber::Publisher publisher(loggerPtr);
    publisher.bind(frontendAddress);
    // Deal with the slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Define message to send
    URTS::MessageFormats::Pick pick;
    pick.setTime(time);
    pick.setNetwork(network);
    pick.setStation(station);
    pick.setChannel(channel);
    pick.setLocationCode(locationCode);
    pick.setPhaseHint(phaseHint);
    pick.setPolarity(polarity);

    for (int i = 0; i < 10; ++i)
    {
        pick.setIdentifier(idBase + i);
        publisher.send(pick);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

void subscriber()
{
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::INFO);
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);
    URTS::Messaging::PublisherSubscriber::Subscriber subscriber(loggerPtr);
    subscriber.connect(backendAddress);
    std::unique_ptr<URTS::MessageFormats::IMessage> pickMessageType
        = std::make_unique<URTS::MessageFormats::Pick> ();
    subscriber.addSubscription(pickMessageType);

    for (int i = 0; i < 10; ++i)
    {
        auto message = subscriber.receive();
        //std::cout << "Got message" << std::endl;
        auto pickMessage
            = static_unique_pointer_cast<URTS::MessageFormats::Pick>
              (std::move(message));
        EXPECT_EQ(pickMessage->getIdentifier(),   idBase + i);
        EXPECT_EQ(pickMessage->getNetwork(),      network);
        EXPECT_EQ(pickMessage->getStation(),      station);
        EXPECT_EQ(pickMessage->getChannel(),      channel);
        EXPECT_EQ(pickMessage->getLocationCode(), locationCode);
        EXPECT_EQ(pickMessage->getPhaseHint(),    phaseHint);
        EXPECT_NEAR(pickMessage->getTime(),       time, 1.e-10);
        EXPECT_EQ(pickMessage->getPolarity(),     polarity);
    }
}

TEST(Messaging, xPubxSubWithProxy)
{
    // Create the proxy then wait before connecting subscribers/publishers
    auto proxyThread  = std::thread(proxy);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Create the consumers so messages aren't created before the recipients
    // are created
    auto subscriberThread1 = std::thread(subscriber);
    auto subscriberThread2 = std::thread(subscriber);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Now create a publishers
    auto publisherThread = std::thread(publisher, 1);
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
 
    proxyThread.join();
    subscriberThread1.join();
    subscriberThread2.join();
    publisherThread.join();
}

}