#include <string>
#include <cmath>
#include <cstring>
#include <vector>
#include <limits>
#include "urts/messageFormats/dataPacket.hpp"
#include "urts/messageFormats/earthworm/traceBuf2.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace URTS::MessageFormats;

using MyTypes = ::testing::Types<double, float>;

template<class T>
class DataPacketTest : public testing::Test
{
public:
    std::vector<double> timeSeries{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int64_t startTimeMuS = 1628803598000000;
    int64_t endTimeMuS = 1628803598000000 + 225000; // 225000 = std::round(9./40*1000000); 
    double samplingRate = 40;
    double tol = std::numeric_limits<double>::epsilon();
protected:
    DataPacketTest() :
        dataPacket(std::make_unique<DataPacket<T>> ()) 
    {   
    }   
    ~DataPacketTest() override = default;
    std::unique_ptr<DataPacket<T>> dataPacket;
};


TYPED_TEST_SUITE(DataPacketTest, MyTypes);

TYPED_TEST(DataPacketTest, DataPacket)
{
    std::string network = "UU";
    std::string station = "FORK";
    std::string channel = "HHZ";
    std::string locationCode = "01";

    auto timeSeries = this->timeSeries;
    auto startTime = this->startTimeMuS;
    auto samplingRate = this->samplingRate;
    auto tol = this->tol;
    auto endTime = this->endTimeMuS;
    this->dataPacket->setNetwork(network);
    this->dataPacket->setStation(station);
    this->dataPacket->setChannel(channel);
    this->dataPacket->setLocationCode(locationCode);
    this->dataPacket->setStartTime(startTime);
    this->dataPacket->setSamplingRate(samplingRate);
    this->dataPacket->setData(timeSeries);
    // Test a copy (this implicitly tests the copy assignment and
    // the original class)
    auto packetCopy = *this->dataPacket;
    // Verify 
    EXPECT_EQ(packetCopy.getMessageType(), "DataPacket");
    EXPECT_EQ(packetCopy.getStartTime(), startTime);
    EXPECT_NEAR(packetCopy.getSamplingRate(), samplingRate, tol);
    EXPECT_EQ(packetCopy.getNetwork(), network);
    EXPECT_EQ(packetCopy.getStation(), station);
    EXPECT_EQ(packetCopy.getChannel(), channel);
    EXPECT_EQ(packetCopy.getLocationCode(), locationCode);
    EXPECT_EQ(packetCopy.getNumberOfSamples(),
              static_cast<int> (timeSeries.size()));
    EXPECT_EQ(packetCopy.getEndTime(), endTime);
    auto traceBack = packetCopy.getData();
    EXPECT_EQ(traceBack.size(), timeSeries.size());
    for (int i = 0; i < static_cast<int> (traceBack.size()); ++i)
    {
        auto res = static_cast<double> (traceBack[i] - timeSeries[i]);
        EXPECT_NEAR(res, 0, tol);
    }

    //std::cout << packetCopy.toJSON(4) << std::endl;
    auto traceCBOR = packetCopy.toCBOR();
    packetCopy.clear();
    EXPECT_EQ(packetCopy.getNumberOfSamples(), 0);
    packetCopy.fromCBOR(traceCBOR);
    EXPECT_EQ(packetCopy.getMessageType(), "DataPacket");
    EXPECT_EQ(packetCopy.getStartTime(), startTime);
    EXPECT_NEAR(packetCopy.getSamplingRate(), samplingRate, tol);
    EXPECT_EQ(packetCopy.getNetwork(), network);
    EXPECT_EQ(packetCopy.getStation(), station);
    EXPECT_EQ(packetCopy.getChannel(), channel);
    EXPECT_EQ(packetCopy.getLocationCode(), locationCode);
    EXPECT_EQ(packetCopy.getNumberOfSamples(),
              static_cast<int> (timeSeries.size()));
    EXPECT_EQ(packetCopy.getEndTime(), endTime);
    traceBack = packetCopy.getData();
    EXPECT_EQ(traceBack.size(), timeSeries.size());
    for (int i = 0; i < static_cast<int> (traceBack.size()); ++i)
    {
        auto res = static_cast<double> (traceBack[i] - timeSeries[i]);
        EXPECT_NEAR(res, 0, tol);
    }
 
}

}
