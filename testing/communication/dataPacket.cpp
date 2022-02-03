#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
#include "umps/proxyBroadcasts/dataPacket/proxy.hpp"
#include "umps/proxyBroadcasts/dataPacket/proxyOptions.hpp"
#include "umps/proxyBroadcasts/proxy.hpp"
#include "umps/proxyBroadcasts/proxyOptions.hpp"
#include "umps/proxyBroadcasts/dataPacket/publisher.hpp"
#include "umps/proxyBroadcasts/dataPacket/publisherOptions.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriber.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxy.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisher.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "private/staticUniquePointerCast.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

const std::string frontendAddress = "tcp://127.0.0.1:5555";
const std::string backendAddress  = "tcp://127.0.0.1:5556";
const std::string network{"UU"};
const std::string station{"FORK"};
const std::string channel{"HHZ"};
const std::string locationCode{"01"};
const double samplingRate{100};
const std::vector<double> timeSeries{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const int nMessages = 10;
const int t0 = 0;

using namespace UMPS::ProxyBroadcasts;

/*
void proxy()
{
    DataPacket::ProxyOptions options;
    options.setName("DataPacket");
    options.setFrontendHighWaterMark(0);
    options.setBackendHighWaterMark(0);
    options.setFrontendAddress(frontendAddress);
    options.setBackendAddress(backendAddress);
    DataPacket::Proxy proxy;
    proxy.initialize(options);
    proxy.start();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(proxy.isRunning());
    std::this_thread::sleep_for(std::chrono::seconds(3));
    proxy.stop();
}
*/

void proxy()
{
    ProxyOptions options;
    options.setName("DataPacket");
    options.setFrontendHighWaterMark(0);
    options.setBackendHighWaterMark(0);
    options.setFrontendAddress(frontendAddress);
    options.setBackendAddress(backendAddress);
    Proxy proxy;
    proxy.initialize(options);
    proxy.start();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(proxy.isRunning());
    std::this_thread::sleep_for(std::chrono::seconds(3));
    proxy.stop();
}

void publisher()
{
    DataPacket::PublisherOptions publisherOptions;
    publisherOptions.setAddress(frontendAddress);
    DataPacket::Publisher publisher; 
    publisher.initialize(publisherOptions);
    EXPECT_TRUE(publisher.isInitialized());
    // Define the mesage to send
    UMPS::MessageFormats::DataPacket<double> packet;
    packet.setNetwork(network);
    packet.setStation(station);
    packet.setChannel(channel);
    packet.setLocationCode(locationCode);
    packet.setSamplingRate(samplingRate);
    packet.setStartTime(t0);
    packet.setData(timeSeries);

    int t0 = 0;
    for (int i = 0; i < nMessages; ++i)
    {
        //std::cout << "send it" << std::endl;
        packet.setStartTime(t0 + i);
        EXPECT_NO_THROW(publisher.send(packet));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void subscriber()
{
    DataPacket::SubscriberOptions subscriberOptions;
    subscriberOptions.setAddress(backendAddress);
    subscriberOptions.setTimeOut(std::chrono::milliseconds(100));
    DataPacket::Subscriber subscriber;
    subscriber.initialize(subscriberOptions);
    EXPECT_TRUE(subscriber.isInitialized());
    int nrecv = 0;
    while (nrecv < nMessages) 
    {
        auto message = subscriber.receive();
        if (message != nullptr)
        {
            EXPECT_EQ(message->getNetwork(), network);
            EXPECT_EQ(message->getStation(), station);
            std::chrono::microseconds startTime{(t0 + nrecv)*1000000};
            EXPECT_EQ(message->getStartTime(), startTime);
            EXPECT_NEAR(message->getSamplingRate(), samplingRate, 1.e-10);
            //std::cout << startTime.count() << " " << message->getStartTime().count() << std::endl;
            nrecv = nrecv + 1;
        }
    }   

}

/*
void baseProxy()
{
    namespace XPubXSub = UMPS::Messaging::XPublisherXSubscriber;
    XPubXSub::ProxyOptions options;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(100);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(200);
    //options.setTopic(topic);
    // Make a logger
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Initialize the server
    XPubXSub::Proxy proxy(loggerPtr);
    proxy.initialize(options);
    // A thread runs the proxy
    std::thread t1(&XPubXSub::Proxy::start,
                   &proxy);
    // Main thread waits...
    std::this_thread::sleep_for(std::chrono::seconds(4));
    /// Main thread tells proxy to stop
    proxy.stop();
    t1.join();
}
*/

/*
void basePublisher()
{
    namespace XPubXSub = UMPS::Messaging::XPublisherXSubscriber;
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    XPubXSub::PublisherOptions options;
    options.setAddress(frontendAddress); 
    XPubXSub::Publisher publisher;
    EXPECT_NO_THROW(publisher.initialize(options));
    EXPECT_TRUE(publisher.isInitialized());
    // Deal with the slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Define the mesage to send
    UMPS::MessageFormats::DataPacket<double> packet;
    packet.setNetwork(network);
    packet.setStation(station);
    packet.setChannel(channel);
    packet.setLocationCode(locationCode);
    packet.setSamplingRate(samplingRate);
    packet.setStartTime(t0);
    packet.setData(timeSeries);
    for (int i = 0; i < nMessages; ++i)
    {   
        //std::chrono::microseconds startTime{t0 + i};
        double startTime = i;
        packet.setStartTime(startTime);
        EXPECT_NO_THROW(publisher.send(packet));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
*/

/*
void baseSubscriber()
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    std::unique_ptr<UMPS::MessageFormats::IMessage> messageType
        = std::make_unique<UMPS::MessageFormats::DataPacket<double>> (); 
    UMPS::MessageFormats::Messages messageTypes;
    messageTypes.add(messageType);

    UMPS::Messaging::PublisherSubscriber::SubscriberOptions options;
    options.setAddress(backendAddress);
    options.setMessageTypes(messageTypes);
    UMPS::Messaging::PublisherSubscriber::Subscriber subscriber(loggerPtr);
    subscriber.initialize(options);
    for (int i = 0; i < nMessages; ++i)
    {
        auto message = subscriber.receive();
    }
}
*/

TEST(Communication, DataPacket)
{
    auto proxyThread = std::thread(proxy);
    //auto proxyThread = std::thread(baseProxy);
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto subThread = std::thread(subscriber);
    //auto subThread = std::thread(baseSubscriber);
    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    auto pubThread = std::thread(publisher);
    //auto pubThread = std::thread(basePublisher);
    
    proxyThread.join();
    //std::cout << "proxy thread joined" << std::endl;
    subThread.join();
    //std::cout << "Sub thread joined" << std::endl;
    pubThread.join();
    //std::cout << "Pub thread joined" << std::endl;
}

}
