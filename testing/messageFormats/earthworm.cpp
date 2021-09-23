#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include "umps/messageFormats/earthworm/traceBuf2.hpp"
#include <gtest/gtest.h>
namespace
{

#define MESSAGE_TYPE "UMPS::MessageFormats::Earthworm::TraceBuf2"

using namespace UMPS::MessageFormats::Earthworm;

using MyTypes = ::testing::Types<double, float, int, int16_t>;

template<class T>
class TraceBuf2Test : public testing::Test
{
public:
    std::vector<double> timeSeries{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    double startTime = 10;
    double samplingRate = 100;
    double tol = std::numeric_limits<double>::epsilon();
    int pinNumber = 2;
    const int sizeT = static_cast<int> (sizeof(T));
protected:
    TraceBuf2Test() :
        traceBuf2(std::make_unique<TraceBuf2<T>> ())
    {
    }
    ~TraceBuf2Test() override = default;
    std::unique_ptr<TraceBuf2<T>> traceBuf2;
};


TYPED_TEST_SUITE(TraceBuf2Test, MyTypes);

TYPED_TEST(TraceBuf2Test, TraceBuf2)
{
    std::string network = "UU";
    std::string station = "FORK";
    std::string channel = "HHZ";
    std::string locationCode = "01";
    int quality = 1;
    auto pinNumber = this->pinNumber;
    auto timeSeries = this->timeSeries;
    auto startTime = this->startTime;
    auto samplingRate = this->samplingRate;
    auto tol = this->tol;
    auto sizeT = this->sizeT;
    this->traceBuf2->setPinNumber(pinNumber);
    this->traceBuf2->setNetwork(network);
    this->traceBuf2->setStation(station);
    this->traceBuf2->setChannel(channel);
    this->traceBuf2->setLocationCode(locationCode);
    this->traceBuf2->setStartTime(startTime);
    this->traceBuf2->setSamplingRate(samplingRate);
    this->traceBuf2->setQuality(quality);
    this->traceBuf2->setData(timeSeries);
    // Test a copy (this implicitly tests the copy assignment and
    // the original class)
    auto traceBuf2Copy = *this->traceBuf2;
    // Verify 
    EXPECT_EQ(traceBuf2Copy.getMessageType(), MESSAGE_TYPE);
    EXPECT_EQ(traceBuf2Copy.getMaximumNumberOfSamples(), (4096 - 64)/sizeT);
    EXPECT_EQ(traceBuf2Copy.getMaximumNetworkLength(), 8);
    EXPECT_EQ(traceBuf2Copy.getMaximumStationLength(), 6);
    EXPECT_EQ(traceBuf2Copy.getMaximumChannelLength(), 3);
    EXPECT_EQ(traceBuf2Copy.getMaximumLocationCodeLength(), 2);
    EXPECT_EQ(traceBuf2Copy.getPinNumber(), pinNumber);
    EXPECT_NEAR(traceBuf2Copy.getStartTime(), startTime, tol);
    EXPECT_NEAR(traceBuf2Copy.getSamplingRate(), samplingRate, tol);
    EXPECT_EQ(traceBuf2Copy.getNetwork(), network);
    EXPECT_EQ(traceBuf2Copy.getStation(), station);
    EXPECT_EQ(traceBuf2Copy.getChannel(), channel);
    EXPECT_EQ(traceBuf2Copy.getLocationCode(), locationCode);
    EXPECT_EQ(traceBuf2Copy.getQuality(), quality);
    EXPECT_EQ(traceBuf2Copy.getNumberOfSamples(),
              static_cast<int> (timeSeries.size()));
    auto tFinal = traceBuf2Copy.getEndTime();
    EXPECT_NEAR(tFinal, startTime + (timeSeries.size() - 1)/samplingRate,
                1.e-14);
    auto traceBack = traceBuf2Copy.getData();
    EXPECT_EQ(traceBack.size(), timeSeries.size());
    for (int i = 0; i < static_cast<int> (traceBack.size()); ++i)
    {
        auto res = static_cast<double> (traceBack[i] - timeSeries[i]);
        EXPECT_NEAR(res, 0, 1.e-14);
    }
    //auto message = traceBuf2Copy.toJSON();

    auto cbor = traceBuf2Copy.toCBOR();
    traceBuf2Copy.clear();
    EXPECT_EQ(traceBuf2Copy.getNumberOfSamples(), 0);
    traceBuf2Copy.fromCBOR(cbor);
    // Verify from CBOR
    EXPECT_EQ(traceBuf2Copy.getMessageType(), MESSAGE_TYPE);
    EXPECT_EQ(traceBuf2Copy.getMaximumNumberOfSamples(), (4096 - 64)/sizeT);
    EXPECT_EQ(traceBuf2Copy.getMaximumNetworkLength(), 8); 
    EXPECT_EQ(traceBuf2Copy.getMaximumStationLength(), 6); 
    EXPECT_EQ(traceBuf2Copy.getMaximumChannelLength(), 3); 
    EXPECT_EQ(traceBuf2Copy.getMaximumLocationCodeLength(), 2); 
    EXPECT_EQ(traceBuf2Copy.getPinNumber(), pinNumber);
    EXPECT_NEAR(traceBuf2Copy.getStartTime(), startTime, tol);
    EXPECT_NEAR(traceBuf2Copy.getSamplingRate(), samplingRate, tol);
    EXPECT_EQ(traceBuf2Copy.getNetwork(), network);
    EXPECT_EQ(traceBuf2Copy.getStation(), station);
    EXPECT_EQ(traceBuf2Copy.getChannel(), channel);
    EXPECT_EQ(traceBuf2Copy.getLocationCode(), locationCode);
    EXPECT_EQ(traceBuf2Copy.getQuality(), quality);
    EXPECT_EQ(traceBuf2Copy.getNumberOfSamples(),
              static_cast<int> (timeSeries.size()));
    tFinal = traceBuf2Copy.getEndTime();
    EXPECT_NEAR(tFinal, startTime + (timeSeries.size() - 1)/samplingRate,
                1.e-14);
    traceBack = traceBuf2Copy.getData();
    EXPECT_EQ(traceBack.size(), timeSeries.size());
    for (int i = 0; i < static_cast<int> (traceBack.size()); ++i)
    {   
        auto res = static_cast<double> (traceBack[i] - timeSeries[i]);
        EXPECT_NEAR(res, 0, 1.e-14);
    }
}

TEST(TraceBuf2Test, FromEarthworm)
{
#define TRACE2_STA_LEN    7    /* SEED: 5 chars plus terminating NULL */
#define TRACE2_NET_LEN    9    /* SEED: 2 chars plus terminating NULL */
#define TRACE2_CHAN_LEN   4    /* SEED: 3 chars plus terminating NULL */
#define TRACE2_LOC_LEN    3    /* SEED: 2 chars plus terminating NULL */
#define MAX_TRACEBUF_SIZ 4096   /* define maximum size of tracebuf message */
    typedef struct
    {
        int     pinno;                 /* Pin number */
        int     nsamp;                 /* Number of samples in packet */
        double  starttime;             /* time of first sample in epoch seconds
                                          (seconds since midnight 1/1/1970) */
        double  endtime;               /* Time of last sample in epoch seconds */
        double  samprate;              /* Sample rate; nominal */
        char    sta[TRACE2_STA_LEN];   /* Site name (NULL-terminated) */
        char    net[TRACE2_NET_LEN];   /* Network name (NULL-terminated) */
        char    chan[TRACE2_CHAN_LEN]; /* Component/channel code (NULL-terminated)*/
        char    loc[TRACE2_LOC_LEN];   /* Location code (NULL-terminated) */
        char    version[2];            /* version field */
        char    datatype[3];           /* Data format code (NULL-terminated) */
        /* quality and pad are available in version 20, see TRACE2X_HEADER */
        char    quality[2];            /* Data-quality field */
        char    pad[2];                /* padding */ 
    } TRACE2_HEADER;
    union
    {
        char          msg[MAX_TRACEBUF_SIZ];
        TRACE2_HEADER trh2;
        //int           i;
    };// TracePacket;
    memset(msg, '\0', sizeof(char)*MAX_TRACEBUF_SIZ);
    int nSamples = 200;
    double starttime = 8;
    double samprate = 100;
    trh2.pinno = 1;
    trh2.nsamp = nSamples;
    trh2.starttime = starttime;
    trh2.endtime = trh2.starttime + (nSamples - 1)/samprate;
    trh2.samprate = samprate;
    strcpy(trh2.sta,      "FAKE"); 
    strcpy(trh2.net,      "FK");
    strcpy(trh2.chan,     "HHZ");
    strcpy(trh2.loc,      "01");
    strcpy(trh2.version,  "20");
    strcpy(trh2.datatype, "f4");
    for (int i = 0; i < nSamples; ++i)
    {
        union
        {
            float f4;
            char c4[4];
        };
        f4 = static_cast<float> (i + 1);
        std::copy(c4, c4 + 4, msg + 64 + 4*i);
    }

    TraceBuf2<double> tb;
    tb.fromEarthworm(msg);
 
    EXPECT_EQ(tb.getNetwork(), "FK");
    EXPECT_EQ(tb.getStation(), "FAKE");
    EXPECT_EQ(tb.getChannel(), "HHZ");
    EXPECT_EQ(tb.getLocationCode(), "01");
    EXPECT_EQ(tb.getVersion(), "20");
    EXPECT_NEAR(tb.getStartTime(),    starttime, 1.e-14);        
    EXPECT_NEAR(tb.getSamplingRate(), samprate,  1.e-14);
    auto data = tb.getData();
    EXPECT_EQ(tb.getNumberOfSamples(), nSamples);
    for (int i = 0; i < nSamples; ++i)
    {
        EXPECT_NEAR(data[i], static_cast<double> (i + 1), 1.e-14);
    } 
/*
auto cb = tb.toCBOR();
auto js = tb.toJSON();
std::cout << cb.size() << " " << js.size() << std::endl;
*/
}

}
