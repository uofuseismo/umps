#include <array>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#ifdef USE_MSEED
#include <libmseed.h>
#endif
extern "C"
{
#include <transport.h>
#include <earthworm_simple_funcs.h>
#include <trace_buf.h>
}
#include "urts/messaging/earthworm/waveRing.hpp"
#include "urts/messageFormats/earthworm/traceBuf2.hpp"
#include "urts/logging/log.hpp"
#include "urts/logging/stdout.hpp"

using namespace URTS::Messaging::Earthworm;

namespace
{
/// This is klunky but effectively earthworm doesn't use const char * which
/// drives C++ nuts.  So we require some fixed size containers to hold 
/// earthworm types.
#ifdef USE_MSEED
std::array<char, 12> TYPE_MSEED{"TYPE_MSEED\0"};
#endif
std::array<char, 12> TYPE_ERROR{"TYPE_ERROR\0"};
std::array<char, 14> MOD_WILDCARD{"MOD_WILDCARD\0"};
std::array<char, 15> INST_WILDCARD{"INST_WILDCARD\0"};
std::array<char, 16> TYPE_HEARTBEAT{"TYPE_HEARTBEAT\0"};
std::array<char, 16> TYPE_TRACEBUF2{"TYPE_TRACEBUF2\0"};
}

class WaveRing::WaveRingImpl
{
public:
    WaveRingImpl(std::shared_ptr<URTS::Logging::ILog> logger) :
        mLogger(logger)
    {
    }
    /// Earthworm messages
    std::vector<URTS::MessageFormats::Earthworm::TraceBuf2<double>>
        mTraceBuf2Messages;
    /// Name of earthworm ring to connect to
    std::shared_ptr<URTS::Logging::ILog> mLogger;
    /// Logos to scrounge from the ring.
    std::vector<MSG_LOGO> mLogos;
    std::string mRingName;
    /// Earthworm shared memory region corresponding to the
    /// earhtworm ring.
    SHM_INFO mRegion;
    long mRingKey = 0;
    uint32_t mMilliSecondsWait = 0;
    /// Earthworm installation ID
    unsigned char mInstallationIdentifier;
    /// Installation wildcard
    unsigned char mInstallationWildcard;
    /// Heartbeat type
    unsigned char mHeartBeatType;
    /// Tracebuffer2 type
    unsigned char mTraceBuffer2Type;
#ifdef USE_MSEED
    /// MSEED type
    unsigned char mMSEEDType;
#endif
    /// Module wildcard
    unsigned char mModWilcard;
    /// Error type
    unsigned char mErrorType;
    /// Have the region?
    bool mHaveRegion = false;
    /// Connected?
    bool mConnected = false;
};

/// C'tor
WaveRing::WaveRing() :
    pImpl(std::make_unique<WaveRingImpl>
          (std::make_shared<URTS::Logging::StdOut> ()))
{
}

/// Move assignment
WaveRing::WaveRing(WaveRing &&waveRing) noexcept
{
    *this = std::move(waveRing);
}

/// Move assignment
WaveRing& WaveRing::operator=(WaveRing &&waveRing) noexcept
{
    if (&waveRing == this){return *this;}
    pImpl = std::move(waveRing.pImpl); 
    return *this;
}

/// Destructor
WaveRing::~WaveRing()
{
    disconnect();
}

/// Disconnects
void WaveRing::disconnect() noexcept
{
    if (pImpl->mHaveRegion){tport_detach(&pImpl->mRegion);}
    memset(&pImpl->mRegion, 0, sizeof(SHM_INFO));
    pImpl->mTraceBuf2Messages.clear();
    pImpl->mLogos.clear();
    pImpl->mRingName.clear();
    pImpl->mMilliSecondsWait = 0;
    pImpl->mConnected = false;
    pImpl->mHaveRegion = false;
}

/// Connect
void WaveRing::connect(const std::string &ringName,
                       const uint32_t wait)
{
    // Get the ring key.  Note, earthworm doesn't believe in const so
    // this is a workaround 
    std::vector<char> ringNameWork(ringName.size() + 1, '\0');
    std::copy(ringName.begin(), ringName.end(), ringNameWork.begin());
    pImpl->mRingKey = GetKey(ringNameWork.data());
    if (pImpl->mRingKey ==-1)
    {
        pImpl->mLogger->error("Failed to get key for ring: " + ringName);
        return;
    }
    // Installation information
    if (GetLocalInst(&pImpl->mInstallationIdentifier) != 0)
    {
    }
    // Various types
    if (GetType(INST_WILDCARD.data(),  &pImpl->mInstallationWildcard) != 0)
    {
        pImpl->mLogger->error("Failed to get installation wildcard");
        throw std::runtime_error("Failed to get installation wildcard");
    }
    if (GetType(TYPE_TRACEBUF2.data(), &pImpl->mTraceBuffer2Type) != 0)
    {
        pImpl->mLogger->error("Failed to get tracebuf2 type");
        throw std::runtime_error("Failed to get tracebuf2 type");
    }
#ifdef USE_MSEED
    if (GetType(TYPE_MSEED.data(), &pImpl->mMSEEDType) != 0)
    {
        pImpl->mLogger->error("Failed to get MSEED type");
        throw std::runtime_error("Failed to get MSEED type");
    }
#endif
    if (GetType(TYPE_HEARTBEAT.data(), &pImpl->mHeartBeatType) != 0)
    {
        pImpl->mLogger->error("Failed to get heartbeat type");
        throw std::runtime_error("Failed to get heartbeat type");
    }
    if (GetType(TYPE_ERROR.data(), &pImpl->mErrorType) != 0)
    {
        pImpl->mLogger->error("Failed to get error type");
        throw std::runtime_error("Failed to get error type");
    }
    if (GetType(MOD_WILDCARD.data(), &pImpl->mModWilcard) != 0)
    {
        pImpl->mLogger->error("Failed to get module ID");
        throw std::runtime_error("Failed to get module ID");
    }
    // Create the logos we wish to read
    pImpl->mLogos.clear();
    pImpl->mLogos.reserve(2);
    MSG_LOGO traceBuf2Logo;
    memset(&traceBuf2Logo, 0, sizeof(MSG_LOGO));
    traceBuf2Logo.type = pImpl->mTraceBuffer2Type;
    pImpl->mLogos.push_back(traceBuf2Logo);
#ifdef USE_MSEED
    MSG_LOGO mseedLogo;
    memset(&mseedLogo, 0, sizeof(MSG_LOGO));
    mseedLogo.type = pImpl->mMSEEDType;
    pImpl->mLogos.push_back(mseedLogo);
#endif
    // Attach to the ring
    tport_attach(&pImpl->mRegion, pImpl->mRingKey);
    pImpl->mHaveRegion = true;
    // Copy some stuff
    pImpl->mRingName = ringName;
    pImpl->mMilliSecondsWait = 0;
    if (wait > 0){pImpl->mMilliSecondsWait = wait;}
    pImpl->mConnected = true;
    // Optimization -> reserve some space
    pImpl->mTraceBuf2Messages.reserve(1024);
}

/// Connected?
bool WaveRing::isConnected() const noexcept
{
    return pImpl->mConnected;
}

/// Reads message from the ring
void WaveRing::read()
{
    if (!isConnected()){throw std::runtime_error("Not to connected to a ring");}
    MSG_LOGO gotLogo;
    std::array<char, MAX_TRACEBUF_SIZ> msg;
    long gotSize = 0;
    int returnCode = 0;
    unsigned char sequenceNumber;
    pImpl->mTraceBuf2Messages.resize(0);
    while(true)
    {
        // Not really sure what to with a kill signal
        auto returnCode = tport_getflag(&pImpl->mRegion);
        if (returnCode == TERMINATE)
        {
            auto error = "Receiving kill signal from ring: " + pImpl->mRingName
                       + "\nDisconneting from ring...";
            pImpl->mLogger->error(error);
            disconnect();
            throw std::runtime_error(error);
        }
        // Copy the ring message
        returnCode = tport_copyfrom(&pImpl->mRegion,
                                    pImpl->mLogos.data(),
                                    pImpl->mLogos.size(),
                                    &gotLogo, &gotSize,
                                    msg.data(), MAX_TRACEBUF_SIZ,
                                    &sequenceNumber);
        if (gotLogo.type == pImpl->mTraceBuffer2Type)
        {
            URTS::MessageFormats::Earthworm::TraceBuf2<double> tb2; 
            try
            {
                tb2.fromEarthworm(msg.data());
            }
            catch (const std::exception &e)
            {
                pImpl->mLogger->error(e.what());
                continue;
            }
            pImpl->mTraceBuf2Messages.push_back(std::move(tb2));
        }
#ifdef USE_MSEED
        else if (gotLogo.type == pImpl->mMSEEDType)
        {
            pImpl->mLogger->error("MSEED message not handled");
            continue;
        }
#endif
    }
    if (pImpl->mMilliSecondsWait > 0){sleep_ew(pImpl->mMilliSecondsWait);}
}

/// Flushes the wave ring
void WaveRing::flush()
{
    if (!isConnected()){throw std::runtime_error("Not to connected to a ring");}
    MSG_LOGO gotLogo;
    std::array<char, MAX_TRACEBUF_SIZ> msg;
    long gotSize = 0;
    int returnCode = 0;
    unsigned char sequenceNumber;
    while (true)
    {
        returnCode = tport_copyfrom(&pImpl->mRegion,
                                    pImpl->mLogos.data(),
                                    pImpl->mLogos.size(),
                                    &gotLogo, &gotSize,
                                    msg.data(), MAX_TRACEBUF_SIZ,
                                    &sequenceNumber);
        if (returnCode == GET_NONE){break;}
    }
    if (pImpl->mMilliSecondsWait > 0){sleep_ew(pImpl->mMilliSecondsWait);}
}

/// Have earthworm?
bool WaveRing::haveEarthworm() const noexcept
{
#ifndef WITH_EARTHWORM
    return true;
#else
    return false;
#endif
}
