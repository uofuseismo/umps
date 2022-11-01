#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
//#include "umps/proxyServices/packetCache/proxy.hpp"
//#include "umps/proxyServices/packetCache/proxyOptions.hpp"
#include "umps/proxyServices/proxy.hpp"
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/replier.hpp"
#include "umps/proxyServices/packetCache/replierOptions.hpp"
#include "umps/proxyServices/packetCache/requestor.hpp"
#include "umps/proxyServices/packetCache/requestorOptions.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/proxyServices/packetCache/sensorRequest.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/dataPacket.hpp"
/*
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
*/
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyServices;
namespace MF = UMPS::MessageFormats; 

const std::string frontendAddress = "tcp://127.0.0.1:5555";
const std::string backendAddress  = "tcp://127.0.0.1:5556";
const std::string serviceName = "PacketCache";
const int maxPackets = 2048;
const std::string network{"UU"};
const std::string station{"FORK"};
const std::string verticalChannel{"HHZ"};
const std::string northChannel{"HHN"};
const std::string eastChannel{"HHE"};
const std::string locationCode{"01"};
const std::vector<std::string> referenceNames{"UU.FORK.HHZ.01",
                                              "UU.FORK.HHE.01",
                                              "UU.FORK.HHN.01"};
const double samplingRate{100};
const std::vector<double> timeSeries{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
/*
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
    std::this_thread::sleep_for(std::chrono::seconds(2));
    proxy.stop();
}

void client()
{
    PacketCache::RequestorOptions options;
    options.setAddress(frontendAddress);
    PacketCache::Requestor client;
    EXPECT_NO_THROW(client.initialize(options));
    EXPECT_TRUE(client.isInitialized());
    PacketCache::SensorRequest sensorRequest;
    sensorRequest.setIdentifier(1);
    auto sensorReply = client.request(sensorRequest);
    if (sensorReply != nullptr)
    {
        EXPECT_EQ(sensorReply->getReturnCode(),
                  PacketCache::ReturnCode::SUCCESS);
        auto names = sensorReply->getNames();
        for (const auto &name : referenceNames)
        {
            EXPECT_TRUE(names.contains(name));
        }
    }
    else
    {
        EXPECT_TRUE(false);
    }
}

void service()
{
    // Make a packet cache
    auto cappedCollection = std::make_shared<PacketCache::CappedCollection<double>> ();
    cappedCollection->initialize(maxPackets);
    MF::DataPacket<double> packet;
    packet.setNetwork(network);
    packet.setStation(station);
    packet.setChannel(verticalChannel); 
    packet.setLocationCode(locationCode);
    packet.setSamplingRate(samplingRate);
    packet.setStartTime(0.0);
    packet.setData(timeSeries);
    cappedCollection->addPacket(packet);
    packet.setChannel(eastChannel);
    cappedCollection->addPacket(packet);
    packet.setChannel(northChannel);
    cappedCollection->addPacket(packet);
    // Make the server
    PacketCache::ReplierOptions options;
    EXPECT_NO_THROW(options.setAddress(backendAddress));
    PacketCache::Replier server;
    EXPECT_NO_THROW(server.initialize(options, cappedCollection));
    EXPECT_NO_THROW(server.start());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_NO_THROW(server.stop());
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
