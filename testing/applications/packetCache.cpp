#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include "urts/applications/packetCache/circularBuffer.hpp"
#include "urts/applications/packetCache/cappedCollection.hpp"
#include "urts/messageFormats/dataPacket.hpp"
#include <gtest/gtest.h>
namespace
{
namespace PC = URTS::Applications::PacketCache;
namespace MF = URTS::MessageFormats;

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

TEST(PacketCache, CircularBufferUpdateOnePacket)
{
std::map<std::string, PC::CircularBuffer<double>> all;
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
    int64_t startTime = 0;
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
        startTime = startTime
                  + static_cast<int64_t> (data.size()*(1000000/samplingRate));
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
    auto t0 = packets[firstPacket-5].getStartTime()*1e-6;
    auto t1 = packets[firstPacket+5].getStartTime()*1e-6;
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
    t0 = packets[firstPacket].getStartTime()*1e-6;
    queryPackets = cb.getPackets(t0, t1);
//std::cout << packets[firstPacket].getStartTime() << " " << t0 << " " << queryPackets[0].getStartTime() << std::endl;
    EXPECT_EQ(queryPackets.size(), 6L); 
    for (int k = 0; k < static_cast<int> (queryPackets.size()); ++k)
    {
        EXPECT_TRUE(queryPackets[k] == packets[firstPacket + k]);
    }
}

}
