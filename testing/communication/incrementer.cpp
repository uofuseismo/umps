#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <filesystem>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
#include "umps/proxyServices/proxy.hpp"
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/proxyServices/incrementer/counter.hpp"
#include "umps/proxyServices/incrementer/replier.hpp"
#include "umps/proxyServices/incrementer/replierOptions.hpp"
#include "umps/proxyServices/incrementer/requestor.hpp"
#include "umps/proxyServices/incrementer/requestorOptions.hpp"
#include "umps/proxyServices/incrementer/itemsRequest.hpp"
#include "umps/proxyServices/incrementer/itemsResponse.hpp"
#include "umps/proxyServices/incrementer/incrementRequest.hpp"
#include "umps/proxyServices/incrementer/incrementResponse.hpp"
#include "umps/proxyServices/incrementer/service.hpp"
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "private/staticUniquePointerCast.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyServices;
namespace MF = UMPS::MessageFormats; 

const std::string frontendAddress = "tcp://127.0.0.1:5555";
const std::string backendAddress  = "tcp://127.0.0.1:5556";
const std::string serviceName = "Incrementer";
//const std::set<std::string> referenceItems = {"Test1", "Test2"};
const std::set<std::string> referenceItems = {"Amplitude",
                                              "Event",
                                              "Magnitude",
                                              "Origin",
                                              "PhasePick",
                                              "PhaseArrival"};

void proxy()
{
    ProxyOptions options;
    options.setName(serviceName);
    options.setFrontendHighWaterMark(0);
    options.setBackendHighWaterMark(0);
    options.setFrontendAddress(frontendAddress);
    options.setBackendAddress(backendAddress);
    Proxy proxy;
    EXPECT_NO_THROW(proxy.initialize(options));
    EXPECT_NO_THROW(proxy.start());
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(proxy.isRunning());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    proxy.stop();
}

void client()
{
    Incrementer::RequestorOptions options;
    options.setAddress(frontendAddress);
    Incrementer::Requestor client;
    EXPECT_NO_THROW(client.initialize(options));
    EXPECT_TRUE(client.isInitialized());
    Incrementer::ItemsRequest itemsRequest;
    itemsRequest.setIdentifier(1);
    // Make sure all the items are there
    auto itemsReply = client.request(itemsRequest);
    if (itemsReply != nullptr)
    {
        EXPECT_EQ(itemsReply->getReturnCode(),
                  Incrementer::ReturnCode::SUCCESS);
        auto names = itemsReply->getItems();
        for (const auto &name : referenceItems)
        {
            EXPECT_TRUE(names.contains(name));
        }
    }
    else
    {
        EXPECT_TRUE(false);
    }
    // 
    Incrementer::IncrementRequest incrementRequest;
    incrementRequest.setItem("Amplitude");
    auto incrementReply = client.request(incrementRequest);
    EXPECT_TRUE(incrementReply->getValue() >= 10 || 
                incrementReply->getValue() <= 15);
    incrementRequest.setItem("Event");
    EXPECT_TRUE(incrementReply->getValue() >= 10 || 
                incrementReply->getValue() <= 15);
    
}

/*
// N.B. This works I just did it the other way to test something
void service()
{
    // Make a packet cache
    auto counter = std::make_shared<Incrementer::Counter> ();
    if (std::filesystem::exists("test_counter.sqlite3"))
    {
        std::filesystem::remove("test_counter.sqlite3");
    } 
    counter->initialize("test_counter.sqlite3");
    int nStart = 1;
    for (const auto &item : referenceItems)
    {
        counter->addItem(item, 5 + nStart, 5);
        nStart = nStart + 1;
    }
    // Make the server
    Incrementer::ReplierOptions options;
    EXPECT_NO_THROW(options.setAddress(backendAddress));
    Incrementer::Replier server;
    EXPECT_NO_THROW(server.initialize(options, counter));
    EXPECT_NO_THROW(server.start());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_NO_THROW(server.stop());
}
*/

TEST(Communication, Incrementer)
{
/*
    auto proxyThread   = std::thread(proxy);
    auto serviceThread = std::thread(service);
    auto clientThread  = std::thread(client);

    clientThread.join();
    serviceThread.join();
    proxyThread.join();
    if (std::filesystem::exists("test_counter.sqlite3"))
    {
        std::filesystem::remove("test_counter.sqlite3");
    }
*/

    auto proxyThread = std::thread(proxy);
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Create a service
    Incrementer::Options options;
    if (std::filesystem::exists("test_counter.sqlite3"))
    {   
        std::filesystem::remove("test_counter.sqlite3");
    }   
    options.setBackendAddress(backendAddress);
    options.setInitialValue(5);
    options.setIncrement(5);
    Incrementer::Service service;
    service.initialize(options);
    service.start();

    auto clientThread  = std::thread(client);

    clientThread.join();
    service.stop();
    proxyThread.join();
}

}
