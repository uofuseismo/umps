#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include "umps/proxyServices/packetCache/bulkDataRequest.hpp"
#include "umps/proxyServices/packetCache/bulkDataResponse.hpp"
#include "umps/proxyServices/packetCache/circularBuffer.hpp"
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/proxyServices/packetCache/wigginsInterpolator.hpp"
#include "umps/proxyServices/packetCache/sensorRequest.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
//#include "umps/proxyServices/packetCache/proxyOptions.hpp"
#include "umps/proxyServices/packetCache/requestorOptions.hpp"
#include "umps/proxyServices/packetCache/replierOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messageFormats/messages.hpp"
#include "private/applications/wiggins.hpp"
#include <gtest/gtest.h>
namespace
{
namespace PC = UMPS::ProxyServices::PacketCache;
namespace MF = UMPS::MessageFormats;
namespace UAuth = UMPS::Authentication;

template<typename T>
bool operator==(const MF::DataPacket<T> &lhs, const MF::DataPacket<T> &rhs)
{
    if (lhs.haveNetwork() == rhs.haveNetwork())
    {
        if (lhs.haveNetwork() && rhs.haveNetwork())
        {
            if (lhs.getNetwork() != rhs.getNetwork()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveStation() == rhs.haveStation())
    {
        if (lhs.haveStation() && rhs.haveStation())
        {
            if (lhs.getStation() != rhs.getStation()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveChannel() == rhs.haveChannel())
    {
        if (lhs.haveChannel() && rhs.haveChannel())
        {
            if (lhs.getChannel() != rhs.getChannel()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveLocationCode() == rhs.haveLocationCode())
    {
        if (lhs.haveLocationCode() && rhs.haveLocationCode())
        {
            if (lhs.getLocationCode() != rhs.getLocationCode()){return false;}
        }
    }
    else
    {
        return false;
    }

    if (lhs.haveSamplingRate() == rhs.haveSamplingRate())
    {
        if (lhs.haveSamplingRate() && rhs.haveSamplingRate())
        {
            if (std::abs(lhs.getSamplingRate()-rhs.getSamplingRate()) > 1.e-14)
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }

    if (lhs.getStartTime() != rhs.getStartTime()){return false;}
    if (lhs.getNumberOfSamples() != rhs.getNumberOfSamples()){return false;}
    if (lhs.getNumberOfSamples() > 0)
    {
        if (lhs.getEndTime() != rhs.getEndTime()){return false;}
        auto lv = lhs.getData();
        auto rv = rhs.getData(); 
        for (int i = 0; i < static_cast<int> (lv.size()); ++i)
        {
            if (std::abs(lv[i] - rv[i]) > std::numeric_limits<T>::epsilon())
            {
                return false;
            }
        }
    }
    return true; 
}

/*
TEST(PacketCache, ProxyOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const std::string backendAddress = "tcp://127.0.0.2:5556";
    const std::string name = "TestPacketCache";
    const int frontendHWM = 100;
    const int backendHWM = 200;
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseServer();
    PC::ProxyOptions options;
    options.setName(name);
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(backendHWM);
    options.setZAPOptions(zapOptions);
    //options.setTopic(topic);
  
    PC::ProxyOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getName(), name);
    EXPECT_EQ(optionsCopy.getFrontendAddress(), frontendAddress);
    EXPECT_EQ(optionsCopy.getBackendAddress(), backendAddress);
    EXPECT_EQ(optionsCopy.getFrontendHighWaterMark(), frontendHWM);
    EXPECT_EQ(optionsCopy.getBackendHighWaterMark(), backendHWM);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    //EXPECT_EQ(optionsCopy.getTopic(), topic);
   
    options.clear();
    EXPECT_EQ(options.getFrontendHighWaterMark(), 1000);
    EXPECT_EQ(options.getBackendHighWaterMark(), 1000);
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::GRASSLANDS);
}
*/
TEST(PacketCache, Wiggins)
{
    const double samplingRate = 200;
    const double targetSamplingRate = 250;
    const double targetSamplingPeriod = 1/targetSamplingRate;
    std::chrono::microseconds gapTolerance{55000};
    auto infl = std::ifstream("data/gse2.txt");
    std::vector<double> t;
    std::vector<double> x;
    t.reserve(12000);
    x.reserve(12000);
    std::string line;
    while (std::getline(infl, line))
    {
        t.push_back(t.size()/samplingRate);
        x.push_back(std::stod(line));
    }
    infl.close();
    EXPECT_EQ(x.size(), 12000);
    std::vector<double> newTimes;
    std::vector<double> yRef;
    newTimes.reserve(14999);
    yRef.reserve(14999);
    infl = std::ifstream("data/wigint.txt");
    while (std::getline(infl, line))
    {
        double t, yi;
        sscanf(line.c_str(), "%lf,%lf\n", &t, &yi);
        newTimes.push_back(t);
        yRef.push_back(yi);
    }
    infl.close();
    EXPECT_EQ(yRef.size(), 14999);
    // Interpolate this
    auto yi = weightedAverageSlopes(t, x, newTimes.front(),
                                    newTimes.back(), targetSamplingRate);
    EXPECT_EQ(yi.size(), yRef.size());
    double yDiff = 0;
    for (int i = 0; i < static_cast<int> (yi.size()); ++i)
    {
        yDiff = std::max(yDiff, std::abs(yi.at(i) - yRef.at(i)));
    }
    EXPECT_NEAR(yDiff, 0, 1.e-8);
    // Packetize this data
    auto n = static_cast<int> (x.size());
    std::vector<UMPS::MessageFormats::DataPacket<double>> packets;
    int packetSize = 100;
    int i0 = 0;
    double t0 = 1644516968;
    std::chrono::microseconds t0MuSec{static_cast<int64_t> (std::round(t0*1000000))};
    double t1 = t0 + (yRef.size() - 1)*targetSamplingPeriod;
    std::chrono::microseconds t1MuSec{static_cast<int64_t> (std::round(t1*1000000))}; 
    for (int i = 0; i < n; ++i)
    {
        UMPS::MessageFormats::DataPacket<double> packet;
        auto i1 = std::min(i0 + packetSize, n);
        auto nCopy = i1 - i0;
        packet.setNetwork("UU");
        packet.setStation("GH2");
        packet.setChannel("EHZ");
        packet.setLocationCode("01");
        packet.setStartTime(t0 + i0/samplingRate);
        packet.setSamplingRate(samplingRate);
        packet.setData(nCopy, &x[i0]);
        packets.push_back(std::move(packet));
        i0 = i1;
        if (i0 == n){break;}
    }
    // Interpolate it
    PC::WigginsInterpolator interpolator;
    EXPECT_NO_THROW(interpolator.setTargetSamplingRate(targetSamplingRate));
    EXPECT_NO_THROW(interpolator.setGapTolerance(gapTolerance));
    EXPECT_NO_THROW(interpolator.interpolate(packets));
    EXPECT_EQ(interpolator.getStartTime(), t0MuSec);
    EXPECT_EQ(interpolator.getEndTime(), t1MuSec);
    EXPECT_EQ(interpolator.getNumberOfSamples(), 
              static_cast<int> (newTimes.size()));
    yi = interpolator.getSignal();
/*
    auto interpolatedPacket = PC::interpolate(packets, targetSamplingRate);
    EXPECT_EQ(interpolatedPacket.getNumberOfSamples(),
              static_cast<int> (newTimes.size()));
    yi = interpolatedPacket.getData();
*/
    yDiff = 0;
    for (int i = 0; i < static_cast<int> (yi.size()); ++i)
    {
        yDiff = std::max(yDiff, std::abs(yi.at(i) - yRef.at(i)));
    }
    EXPECT_NEAR(yDiff, 0, 1.e-8);
    // Try clearing the signal
    interpolator.clearSignal();
    EXPECT_NEAR(interpolator.getTargetSamplingRate(),
                targetSamplingRate, 1.e-14);
    EXPECT_EQ(interpolator.getNumberOfSamples(), 0);
    EXPECT_EQ(interpolator.getGapTolerance(), gapTolerance);

    // Add some duplicate packets (really wherever)
    packets.push_back(packets[0]);
    packets.push_back(packets[1]);
    // And change the order
    std::srand(500582);
    std::random_shuffle(packets.begin(), packets.end());
    EXPECT_NO_THROW(interpolator.interpolate(packets));
    EXPECT_EQ(interpolator.getStartTime(), t0MuSec);
    EXPECT_EQ(interpolator.getEndTime(), t1MuSec);
    EXPECT_EQ(interpolator.getNumberOfSamples(),
              static_cast<int> (newTimes.size()));
    yi = interpolator.getSignal();
/*
    EXPECT_NO_THROW(interpolatedPacket
        = PC::interpolate(packets, targetSamplingRate));
    EXPECT_EQ(interpolatedPacket.getNumberOfSamples(),
              static_cast<int> (newTimes.size()));
    yi = interpolatedPacket.getData();
*/
    yDiff = 0;
    for (int i = 0; i < static_cast<int> (yi.size()); ++i)
    {
        yDiff = std::max(yDiff, std::abs(yi.at(i) - yRef.at(i)));
    }
    EXPECT_NEAR(yDiff, 0, 1.e-8);
}

TEST(PacketCache, RequestOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    //const std::string topic = "testTopic";
    const int hwm = 100;
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    PC::RequestorOptions options;
    options.setAddress(frontendAddress);
    options.setHighWaterMark(hwm);
    options.setZAPOptions(zapOptions);

    PC::RequestorOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), frontendAddress);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), hwm);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    auto messages = optionsCopy.getRequestOptions().getMessageFormats();
    PC::SensorResponse sensorResponse;
    PC::DataResponse<double> dataResponse;
    EXPECT_TRUE(messages.contains(sensorResponse.getMessageType()));
    EXPECT_TRUE(messages.contains(dataResponse.getMessageType()));

    

    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), 2048);//1024);
}

TEST(PacketCache, ReplierOptions)
{
    const std::string address = "tcp://localhost:5050";
    const int hwm = 1234;
    PC::ReplierOptions replier;
    replier.setAddress(address);
    replier.setHighWaterMark(hwm);
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    replier.setZAPOptions(zapOptions);

    PC::ReplierOptions optionsCopy(replier);
    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    EXPECT_EQ(optionsCopy.getHighWaterMark(), hwm);
}

TEST(PacketCache, SensorRequest)
{
    PC::SensorRequest request;
    const uint64_t id = 600238; 
    request.setIdentifier(id);
    EXPECT_EQ(request.getMessageType(),
              "UMPS::ProxyServices::PacketCache::SensorRequest");
 
    auto message = request.toMessage();
    PC::SensorRequest requestCopy;
    EXPECT_NO_THROW(requestCopy.fromMessage(message)); //message.data(), message.size());
    EXPECT_EQ(requestCopy.getIdentifier(), id);
}

TEST(PacketCache, SensorResponse)
{
    const std::unordered_set<std::string> names{"UU.FORK.HHN.01",
                                                "UU.FORK.HHE.01",
                                                "UU.FORK.HHZ.01",
                                                "WY.YFT.EHZ.01"};
    PC::SensorResponse response;
    PC::ReturnCode rc = PC::ReturnCode::INVALID_MESSAGE;
    const uint64_t id = 600238; 
    response.setIdentifier(id);
    response.setReturnCode(rc);
    EXPECT_NO_THROW(response.setNames(names));
    EXPECT_EQ(response.getMessageType(),
              "UMPS::ProxyServices::PacketCache::SensorResponse");
 
    auto message = response.toMessage();
    PC::SensorResponse responseCopy;
    EXPECT_NO_THROW(responseCopy.fromMessage(message)); //message.data(), message.size());
    EXPECT_EQ(responseCopy.getIdentifier(), id); 
    EXPECT_EQ(responseCopy.getReturnCode(), rc);
    // There's really no guarantee how the names come back
    auto namesBack = responseCopy.getNames();
    EXPECT_EQ(namesBack.size(), names.size());
    for (const auto &n : namesBack)
    {
        EXPECT_TRUE(names.contains(n));
    }

    // Make sure I can deal with no names
    response.clear();
    message = response.toMessage();
    responseCopy.fromMessage(message.data(), message.size());
    EXPECT_EQ(response.getIdentifier(), 0);
    EXPECT_EQ(response.getReturnCode(), PC::ReturnCode::SUCCESS);
    namesBack = responseCopy.getNames();
    EXPECT_TRUE(namesBack.empty());
}

TEST(PacketCache, DataRequest)
{
    PC::DataRequest request;
    const std::string network = "UU";
    const std::string station = "ARUT";
    const std::string channel = "EHZ";
    const std::string locationCode = "01";
    const uint64_t id = 400038;
    double t0 = 1629737861;
    double t1 = 1629737865;
    EXPECT_EQ(request.getMessageType(),
              "UMPS::ProxyServices::PacketCache::DataRequest");
    EXPECT_NO_THROW(request.setNetwork(network));
    EXPECT_NO_THROW(request.setStation(station));
    EXPECT_NO_THROW(request.setChannel(channel));
    EXPECT_NO_THROW(request.setLocationCode(locationCode));
    request.setIdentifier(id);
    request.setQueryTime(t0); 
    auto [qTimeStart, qTimeEnd] = request.getQueryTimes();
    EXPECT_NEAR(qTimeStart, t0, 1.e-5);
    EXPECT_EQ(qTimeEnd, std::numeric_limits<uint32_t>::max()); //double>::max());
    EXPECT_NO_THROW(request.setQueryTimes(std::pair(t0, t1)));

    auto message = request.toMessage();
    PC::DataRequest requestCopy;
    EXPECT_NO_THROW(requestCopy.fromMessage(message)); //message.data(), message.size());
    EXPECT_EQ(requestCopy.getNetwork(), network);
    EXPECT_EQ(requestCopy.getStation(), station);
    EXPECT_EQ(requestCopy.getChannel(), channel);
    EXPECT_EQ(requestCopy.getLocationCode(), locationCode);
    EXPECT_EQ(requestCopy.getIdentifier(), id);
    auto [timeStart, timeEnd] = requestCopy.getQueryTimes();
    EXPECT_NEAR(timeStart, t0, 1.e-5);
    EXPECT_NEAR(timeEnd,   t1, 1.e-5);
}

TEST(PacketCache, BulkDataRequest)
{
    PC::BulkDataRequest bulkRequest;
    const std::string network = "UU";
    const std::string station = "ARUT";
    const std::vector<std::string> channels{"EHZ", "EHN", "EHE"};
    int nRequests = static_cast<int> (channels.size());
    const std::string locationCode = "01";
    const uint64_t id = 400038;
    double t0 = 1629737861;
    double t1 = 1629737865;
    EXPECT_EQ(bulkRequest.getMessageType(),
              "UMPS::ProxyServices::PacketCache::BulkDataRequest");

    PC::DataRequest request;
    EXPECT_NO_THROW(request.setNetwork(network));
    EXPECT_NO_THROW(request.setStation(station));
    EXPECT_NO_THROW(request.setLocationCode(locationCode));
    request.setQueryTimes(std::pair {t0, t1}); 
    for (int i = 0; i < nRequests; ++i)
    {
        EXPECT_NO_THROW(request.setChannel(channels.at(i)));
        request.setIdentifier(id + i);
        bulkRequest.addDataRequest(request);
    }
    bulkRequest.setIdentifier(id);
    // Check for a duplicate - this is the most common case
    request.setIdentifier(id + 0);
    request.setChannel(channels.at(0));
    EXPECT_THROW(bulkRequest.addDataRequest(request), std::invalid_argument);

    EXPECT_EQ(bulkRequest.getNumberOfDataRequests(), nRequests);

    auto message = bulkRequest.toMessage();
    PC::BulkDataRequest bulkRequestCopy;
    EXPECT_NO_THROW(bulkRequestCopy.fromMessage(message)); //message.data(), message.size());

    EXPECT_EQ(bulkRequestCopy.getNumberOfDataRequests(), nRequests);
    EXPECT_EQ(bulkRequestCopy.getIdentifier(), id);
    auto requestsPtr = bulkRequestCopy.getDataRequestsPointer();
    for (int i = 0; i < nRequests; ++i)
    {
        EXPECT_EQ(requestsPtr[i].getNetwork(), network);
        EXPECT_EQ(requestsPtr[i].getStation(), station);
        EXPECT_EQ(requestsPtr[i].getChannel(), channels.at(i));
        EXPECT_EQ(requestsPtr[i].getLocationCode(), locationCode);
        EXPECT_EQ(requestsPtr[i].getIdentifier(), id + i);
        auto [timeStart, timeEnd] = requestsPtr[i].getQueryTimes();
        EXPECT_NEAR(timeStart, t0, 1.e-5);
        EXPECT_NEAR(timeEnd,   t1, 1.e-5);
    }
}

TEST(PacketCache, DataResponse)
{
    const std::string network{"UU"};
    const std::string station{"VRUT"};
    const std::string channel{"EHZ"};
    const std::string locationCode{"01"};
    const double samplingRate = 100;
    const uint64_t id = 594382;
    std::vector<UMPS::MessageFormats::DataPacket<double>> dataPackets;
    const double t0 = 0;
    std::vector<double> startTimes;
    const std::vector<int> samplesPerPacket{100, 200, 100, 200};
    auto t1 = t0;
    for (const auto &nSamples : samplesPerPacket)
    {
        startTimes.push_back(t1);
        UMPS::MessageFormats::DataPacket<double> dataPacket;
        dataPacket.setNetwork(network);
        dataPacket.setStation(station);
        dataPacket.setChannel(channel);
        dataPacket.setLocationCode(locationCode);
        dataPacket.setSamplingRate(samplingRate);
        dataPacket.setStartTime(t1);
        std::vector<double> data(nSamples);
        std::fill(data.begin(), data.end(), static_cast<double> (nSamples));
        dataPacket.setData(data);
        dataPackets.push_back(dataPacket);
        // Update start time
        t1 = t1 + std::round( (nSamples - 1)/samplingRate );
    }
    PC::DataResponse<double> response;
    PC::ReturnCode rc = PC::ReturnCode::INVALID_MESSAGE;
    EXPECT_NO_THROW(response.setPackets(dataPackets));
    EXPECT_NO_THROW(response.setIdentifier(id));
    EXPECT_NO_THROW(response.setReturnCode(rc));
   
    // Reconstitute the class from a message 
    auto message = response.toMessage(); 
    PC::DataResponse<double> responseCopy;
    EXPECT_NO_THROW(responseCopy.fromMessage(message)); //message.data(), message.size()));
    EXPECT_EQ(responseCopy.getIdentifier(), id);
    EXPECT_EQ(responseCopy.getReturnCode(), rc);
    EXPECT_EQ(responseCopy.getMessageType(), 
              "UMPS::ProxyServices::PacketCache::DataResponse");
    auto packetsBack = responseCopy.getPackets();
    EXPECT_EQ(packetsBack.size(), dataPackets.size());
    for (size_t i = 0; i < packetsBack.size(); ++i)
    {
        EXPECT_TRUE(packetsBack.at(i) == dataPackets.at(i));
    }

    // Set reversed packets
    std::reverse(packetsBack.begin(), packetsBack.end());
    response.setPackets(packetsBack);
    message = response.toMessage();
    EXPECT_NO_THROW(responseCopy.fromMessage(message)); //message.data(), message.size());
    packetsBack = responseCopy.getPackets();
    for (size_t i = 0; i < packetsBack.size(); ++i)
    {
        EXPECT_TRUE(packetsBack.at(i) == dataPackets.at(i));
    }
   
    // See what happens when multiple packets start at same time.
    // This shouldn't result in a sort.
    dataPackets[0].setStartTime(0);
    dataPackets[1].setStartTime(0);
    EXPECT_NO_THROW(responseCopy.setPackets(dataPackets));
    packetsBack = responseCopy.getPackets();
    for (size_t i = 0; i < packetsBack.size(); ++i)
    {   
        EXPECT_TRUE(packetsBack.at(i) == dataPackets.at(i));
    }
}

TEST(PacketCache, BulkDataResponse)
{
    const std::string network{"UU"};
    const std::string station{"VRUT"};
    const std::vector<std::string> channels{"EHZ", "EHN", "EHE"};
    const std::string locationCode{"01"};
    const double samplingRate = 100;
    const uint64_t id = 594382;
    std::vector<UMPS::MessageFormats::DataPacket<double>>
       zDataPackets, nDataPackets, eDataPackets;
    const double t0 = 0;
    std::vector<double> startTimes;
    const std::vector<int> samplesPerPacket{100, 200, 100, 200};
    auto t1 = t0;
    for (const auto &nSamples : samplesPerPacket)
    {
        startTimes.push_back(t1);
        UMPS::MessageFormats::DataPacket<double> dataPacket;
        dataPacket.setNetwork(network);
        dataPacket.setStation(station);
        dataPacket.setChannel(channels.at(0));
        dataPacket.setLocationCode(locationCode);
        dataPacket.setSamplingRate(samplingRate);
        dataPacket.setStartTime(t1);
        std::vector<double> data(nSamples); 
        std::fill(data.begin(), data.end(), static_cast<double> (nSamples));
        dataPacket.setData(data);
        zDataPackets.push_back(dataPacket);
 
        dataPacket.setChannel(channels.at(1));
        nDataPackets.push_back(dataPacket);

        dataPacket.setChannel(channels.at(2));
        eDataPackets.push_back(dataPacket);
        // Update start time
        t1 = t1 + std::round( (nSamples - 1)/samplingRate );
    }
    PC::BulkDataResponse<double> bulkResponse;
    PC::DataResponse<double> response;
    PC::ReturnCode rc = PC::ReturnCode::INVALID_MESSAGE;
    response.setPackets(zDataPackets);
    response.setIdentifier(id + 1);
    response.setReturnCode(rc);
    EXPECT_NO_THROW(bulkResponse.addDataResponse(response));

    response.setPackets(nDataPackets);
    response.setIdentifier(id + 2);
    response.setReturnCode(rc);
    EXPECT_NO_THROW(bulkResponse.addDataResponse(response));

    response.setPackets(eDataPackets);
    response.setIdentifier(id + 3);
    response.setReturnCode(rc);
    EXPECT_NO_THROW(bulkResponse.addDataResponse(response));

    EXPECT_EQ(bulkResponse.getNumberOfDataResponses(), 3);
    bulkResponse.setIdentifier(id);
    bulkResponse.setReturnCode(PC::ReturnCode::NO_SENSOR);

    // Reconsitute the bulk response
    auto message = bulkResponse.toMessage();
    PC::BulkDataResponse brCopy;
    EXPECT_NO_THROW(brCopy.fromMessage(message)); //message.data(), message.size());

    EXPECT_EQ(brCopy.getMessageType(),
              "UMPS::ProxyServices::PacketCache::BulkDataResponse"); 
    EXPECT_EQ(brCopy.getReturnCode(), PC::ReturnCode::NO_SENSOR);
    EXPECT_EQ(brCopy.getNumberOfDataResponses(), 3);
    EXPECT_EQ(brCopy.getIdentifier(), id);
    auto responses = brCopy.getDataResponses();
    int i = 0;
    for (const auto &r : responses)
    {
        EXPECT_EQ(r.getReturnCode(), PC::ReturnCode::INVALID_MESSAGE);
        EXPECT_EQ(r.getIdentifier(), id + i + 1);
        auto packetsBack = r.getPackets();
        EXPECT_EQ(packetsBack.size(), samplesPerPacket.size());
        if (i == 0)
        {
            for (int j = 0; j < static_cast<int> (packetsBack.size()); ++j)
            {
                EXPECT_TRUE(packetsBack.at(j) == zDataPackets.at(j));
            }
        }
        else if (i == 1)
        {
            for (int j = 0; j < static_cast<int> (packetsBack.size()); ++j)
            {
                EXPECT_TRUE(packetsBack.at(j) == nDataPackets.at(j));
            }
        }
        else if (i == 2)
        {
            for (int j = 0; j < static_cast<int> (packetsBack.size()); ++j)
            {
                EXPECT_TRUE(packetsBack.at(j) == eDataPackets.at(j));
            }
        }
        else
        {
            ASSERT_TRUE(false);
        }
        i = i + 1;
    }

}

TEST(PacketCache, CircularBuffer)
{
    PC::CircularBuffer<double> cb;
    const std::string network{"UU"};
    const std::string station{"TCU"};
    const std::string channel{"EHZ"};
    const std::string locationCode{"01"};
    const int packetSize = 100;
    const double samplingRate = 100;
    const int maxPackets = 10;
    const int nPacketsInVector = 20;
    cb.initialize(network, station, channel, locationCode, maxPackets);
    EXPECT_TRUE(cb.isInitialized());
    EXPECT_EQ(cb.getNetwork(), network);
    EXPECT_EQ(cb.getStation(), station);
    EXPECT_EQ(cb.getChannel(), channel);
    EXPECT_EQ(cb.getLocationCode(), locationCode);
    EXPECT_EQ(cb.getMaximumNumberOfPackets(), maxPackets);
    EXPECT_EQ(cb.getNumberOfPackets(), 0);
    
    // Make a bunch of data samples in order to track our progress
    std::vector<MF::DataPacket<double>> packets(nPacketsInVector);
    int index = 0;
    double startTime = 0;
    for (int ip = 0; ip < static_cast<int> (packets.size()); ++ip)
    {
        packets[ip].setNetwork(network);
        packets[ip].setStation(station);
        packets[ip].setChannel(channel);
        packets[ip].setLocationCode(locationCode);
        packets[ip].setSamplingRate(samplingRate);
        packets[ip].setStartTime(startTime);
   
        std::vector<int> data(packetSize);
        std::iota(data.begin(), data.end(), index);
        packets[ip].setData(data);

        index = index + packetSize;
        startTime = startTime + data.size()/samplingRate;
//                  + static_cast<int64_t> (data.size()*(1000000/samplingRate));
        //std::cout << packets[ip].getEndTime() << " " << startTime << std::endl;
    }
    // Add a packet
    cb.addPacket(packets[1]);
    EXPECT_EQ(cb.getNumberOfPackets(), 1);
    // Overwrite myself
    cb.addPacket(packets[1]);
    EXPECT_EQ(cb.getNumberOfPackets(), 1);
    // Add out of order packet
    cb.addPacket(packets[0]);
    EXPECT_EQ(cb.getNumberOfPackets(), 2);
    EXPECT_EQ(packets[0].getStartTime(), cb.getEarliestStartTime());
    // Add them all
    for (const auto &packet : packets)
    {
        cb.addPacket(packet);
    }
    // Circular buffer should be full now
    EXPECT_EQ(cb.getNumberOfPackets(), cb.getMaximumNumberOfPackets());
    int firstPacket = 0;
    for (int k = 0; k < std::numeric_limits<int>::max(); ++k)
    {
        if (firstPacket + maxPackets >= nPacketsInVector){break;}
        firstPacket = firstPacket + maxPackets;
    }
    EXPECT_EQ(cb.getEarliestStartTime(),
              packets.at(firstPacket).getStartTime());
    // Get all the packets in the buffer
    auto allPackets = cb.getPackets();
    for (int k = 0; k < static_cast<int> (allPackets.size()); ++k)
    {
        EXPECT_TRUE(allPackets[k] == packets[firstPacket + k]);
    }
    // Do a query from before the first packet to somewhere before the end
    auto t0 = packets[firstPacket-5].getStartTime().count()*1e-6;
    auto t1 = packets[firstPacket+5].getStartTime().count()*1e-6;
    auto queryPackets = cb.getPackets(t0, t1);
    EXPECT_EQ(queryPackets.size(), 6L); // 0, 1, 2, 3, 4, 5 
    for (int k = 0; k < static_cast<int> (queryPackets.size()); ++k)
    {
        EXPECT_TRUE(queryPackets[k] == packets[firstPacket + k]);
    }
    // Get all the packets from t0 to now
    queryPackets = cb.getPackets(t0);
    EXPECT_EQ(queryPackets.size(), allPackets.size());
    for (int k = 0; k < static_cast<int> (queryPackets.size()); ++k)
    {
        EXPECT_TRUE(queryPackets[k] == packets[firstPacket + k]);
    }
    // Do a query from the first packet to now to check equality
    t0 = packets[firstPacket].getStartTime().count()*1e-6;
    queryPackets = cb.getPackets(t0, t1);
//std::cout << packets[firstPacket].getStartTime() << " " << t0 << " " << queryPackets[0].getStartTime() << std::endl;
    EXPECT_EQ(queryPackets.size(), 6L); 
    for (int k = 0; k < static_cast<int> (queryPackets.size()); ++k)
    {
        EXPECT_TRUE(queryPackets[k] == packets[firstPacket + k]);
    }
    // Do a query from just after the first packet to just before the
    // last packet .  If we can, we want to get the first.
    t0 = packets[firstPacket].getStartTime().count()*1e-6 + 1.e-6;
    t1 = packets.back().getStartTime().count()*1.e-6 - 1.e-6;
    queryPackets = cb.getPackets(t0, t1);
    EXPECT_EQ(queryPackets.size(), packets.size() - firstPacket - 1);//2);
    for (int k = 0; k < static_cast<int> (queryPackets.size()); ++k)
    {
        EXPECT_TRUE(queryPackets[k] == packets[firstPacket + k]);// + 1*0]); 
    }
}

}
