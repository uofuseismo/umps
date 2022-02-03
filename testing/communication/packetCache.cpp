#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
//#include "umps/proxyServices/packetCache/proxy.hpp"
//#include "umps/proxyServices/packetCache/proxyOptions.hpp"
#include "umps/proxyServices/proxy.hpp"
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/proxyServices/packetCache/reply.hpp"
#include "umps/proxyServices/packetCache/replyOptions.hpp"
#include "umps/proxyServices/packetCache/request.hpp"
#include "umps/proxyServices/packetCache/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/dataPacket.hpp"
/*
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
*/
#include "private/staticUniquePointerCast.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyServices;

const std::string frontendAddress = "tcp://127.0.0.1:5555";
const std::string backendAddress  = "tcp://127.0.0.1:5556";
const std::string serviceName = "PacketCache";
/*
const std::string network{"UU"};
const std::string station{"FORK"};
const std::string channel{"HHZ"};
const std::string locationCode{"01"};
const double samplingRate{100};
const std::vector<double> timeSeries{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const int nMessages = 10;
const int t0 = 0;
*/

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
    std::this_thread::sleep_for(std::chrono::seconds(3));
    proxy.stop();
}

void client()
{
    PacketCache::RequestOptions options;
    options.setAddress(frontendAddress);
    PacketCache::Request client;
    EXPECT_NO_THROW(client.initialize(options));
    EXPECT_TRUE(client.isInitialized());
}

void service()
{
    // Make a packet cache
    PacketCache::ReplyOptions options;
    options.setAddress(backendAddress);
    PacketCache::Reply server;

}

TEST(Communication, PacketCache)
{
    auto proxyThread   = std::thread(proxy);
    auto serviceThread = std::thread(service);
    auto clientThread  = std::thread(client);

    clientThread.join();
    serviceThread.join();
    proxyThread.join();
}

}
