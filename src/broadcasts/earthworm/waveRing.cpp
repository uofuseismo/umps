#include <iostream>
#include <chrono>
#include <array>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#ifdef WITH_TBB
#include <tbb/parallel_for.h>
#endif
#undef WITH_MSEED
#ifdef WITH_MSEED
#include <libmseed.h>
#endif
#ifdef WITH_EARTHWORM
extern "C"
{
#include <transport.h>
#include <earthworm_simple_funcs.h>
#include <trace_buf.h>
}
#endif
#include "umps/broadcasts/earthworm/waveRing.hpp"
#include "umps/broadcasts/earthworm/traceBuf2.hpp"
#include "umps/logging/log.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Broadcasts::Earthworm;

namespace
{
/// This is klunky but effectively earthworm doesn't use const char * which
/// drives C++ nuts.  So we require some fixed size containers to hold 
/// earthworm types.
#ifdef WITH_MSEED
std::array<char, 12> TYPE_MSEED{"TYPE_MSEED\0"};
#endif
std::array<char, 12> TYPE_ERROR{"TYPE_ERROR\0"};
std::array<char, 14> MOD_WILDCARD{"MOD_WILDCARD\0"};
std::array<char, 15> INST_WILDCARD{"INST_WILDCARD\0"};
std::array<char, 16> TYPE_HEARTBEAT{"TYPE_HEARTBEAT\0"};
std::array<char, 16> TYPE_TRACEBUF2{"TYPE_TRACEBUF2\0"};
//std::array<char, 21> TYPE_TRACECOMP2{"TYPE_TRACE2_COMP_UA\0"};
}

class WaveRing::WaveRingImpl
{
public:
    WaveRingImpl(std::shared_ptr<UMPS::Logging::ILog> logger) :
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }  
    }
    /// Earthworm messages
    std::vector<TraceBuf2<double>> mTraceBuf2Messages;
    /// Name of earthworm ring to connect to
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    /// Logos to scrounge from the ring.
    std::vector<MSG_LOGO> mLogos;
    std::string mRingName;
    /// Earthworm shared memory region corresponding to the
    /// earhtworm ring.
    SHM_INFO mRegion;
    long mRingKey = 0;
    uint32_t mMilliSecondsWait = 0;
    /// Earthworm installation ID
    unsigned char mInstallationIdentifier = 0;
    /// Installation wildcard
    unsigned char mInstallationWildCard = 0;
    /// Heartbeat type
    unsigned char mHeartBeatType = 0;
    /// Tracebuffer2 type
    unsigned char mTraceBuffer2Type = 0;
    /// TraceComp2
    //unsigned char mTraceComp2Type = 0;
#ifdef WITH_MSEED
    /// MSEED type
    unsigned char mMSEEDType = 0;
#endif
    /// Module wildcard
    unsigned char mModWildCard = 0;
    /// Error type
    unsigned char mErrorType = 0;
    /// Most waves read off the ring
    int mMostWavesRead = 0;
    /// Have the region?
    bool mHaveRegion = false;
    /// Connected?
    bool mConnected = false;
};

/// C'tor
WaveRing::WaveRing() :
    pImpl(std::make_unique<WaveRingImpl>
          (std::make_shared<UMPS::Logging::StdOut> ()))
{
}

/// C'tor with logger
WaveRing::WaveRing(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<WaveRingImpl> (logger))
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
#ifdef WITH_EARTHWORM
    pImpl->mLogger->debug("Disconnecting...");
    if (pImpl->mHaveRegion){tport_detach(&pImpl->mRegion);}
    memset(&pImpl->mRegion, 0, sizeof(SHM_INFO));
    pImpl->mTraceBuf2Messages.clear();
    pImpl->mLogos.clear();
    pImpl->mRingName.clear();
    pImpl->mRingKey = 0;
    pImpl->mMilliSecondsWait = 0;
    pImpl->mInstallationIdentifier = 0;
    pImpl->mInstallationWildCard = 0;
    pImpl->mHeartBeatType = 0;
    pImpl->mTraceBuffer2Type = 0;
    //pImpl->mTraceComp2Type = 0;
#ifdef WITH_MSEED
    pImpl->mMSEEDType = 0;
#endif
    pImpl->mModWildCard = 0;
    pImpl->mErrorType = 0;
    pImpl->mMostWavesRead = 0;
    pImpl->mHaveRegion = false;
    pImpl->mConnected = false;
#endif
}

/// Connect
void WaveRing::connect(const std::string &ringName,
                       const uint32_t wait)
{
    // Checks
    if (!haveEarthworm()){throw std::runtime_error("Recompile with earthworm");}
    if (ringName.empty()){throw std::invalid_argument("ringName is empty");}
    // Make sure I'm not already connected
    disconnect();
#ifdef WITH_EARTHWORM
    // Get the ring key.  Note, earthworm doesn't believe in const so
    // this is a workaround 
    pImpl->mLogger->debug("Getting key from ring: " + ringName);
    std::vector<char> ringNameWork(ringName.size() + 1, '\0');
    std::copy(ringName.begin(), ringName.end(), ringNameWork.begin());
    pImpl->mRingKey = GetKey(ringNameWork.data());
    // Attach to the ring
    pImpl->mLogger->debug("Attaching to ring...");
    tport_attach(&pImpl->mRegion, pImpl->mRingKey);
    pImpl->mHaveRegion = true;
    if (pImpl->mRingKey ==-1)
    {
        pImpl->mLogger->error("Failed to get key for ring: " + ringName);
        return;
    }
    // Installation information
    pImpl->mLogger->debug("Specifying logos...");
    if (GetLocalInst(&pImpl->mInstallationIdentifier) != 0)
    {
        pImpl->mLogger->error("Failed to get installation identifier");
        throw std::runtime_error("Failed to get installation identifier");
    }
    // Various types
    if (GetType(TYPE_TRACEBUF2.data(), &pImpl->mTraceBuffer2Type) != 0)
    {
        pImpl->mLogger->error("Failed to get tracebuf2 type");
        throw std::runtime_error("Failed to get tracebuf2 type");
    }
/*
    if (GetType(TYPE_TRACECOMP2.data(), &pImpl->mTraceComp2Type) != 0)
    {
        pImpl->mLogger->error("Failed to get tracecomp2 type");
        throw std::runtime_error("Failed to get tracecomp2 type");
    }
*/
#ifdef WITH_MSEED
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
    // Wildcard info
    if (GetInst(INST_WILDCARD.data(), &pImpl->mInstallationWildCard) != 0)
    {
        pImpl->mLogger->error("Failed to get installation wildcard");
        throw std::runtime_error("Failed to get installation wildcard");
    }
    if (GetModId(MOD_WILDCARD.data(), &pImpl->mModWildCard) != 0)
    {
        pImpl->mLogger->error("Failed to get module ID");
        throw std::runtime_error("Failed to get module ID");
    }
    // Create the logos we wish to read
    pImpl->mLogos.clear();
    MSG_LOGO traceBuf2Logo;
    memset(&traceBuf2Logo, 0, sizeof(MSG_LOGO));
    traceBuf2Logo.type = pImpl->mTraceBuffer2Type;
    pImpl->mLogos.push_back(traceBuf2Logo);

/*
    MSG_LOGO traceComp2Logo;
    memset(&traceComp2Logo, 0, sizeof(MSG_LOGO));
    traceComp2Logo.type = pImpl->mTraceComp2Type;
    pImpl->mLogos.push_back(traceComp2Logo);
*/

#ifdef WITH_MSEED
    MSG_LOGO mseedLogo;
    memset(&mseedLogo, 0, sizeof(MSG_LOGO));
    mseedLogo.type = pImpl->mMSEEDType;
    pImpl->mLogos.push_back(mseedLogo);
#endif
    // Copy some stuff now that we have survived
    pImpl->mRingName = ringName;
    pImpl->mMilliSecondsWait = 0;
    if (wait > 0){pImpl->mMilliSecondsWait = wait;}
    pImpl->mConnected = true;
    // Optimization -> reserve some space
    pImpl->mTraceBuf2Messages.reserve(1024);
    pImpl->mLogger->debug("Connected!");
#endif
}

/// Connected?
bool WaveRing::isConnected() const noexcept
{
    return pImpl->mConnected;
}

/// Reads message from the ring
void WaveRing::read()
{
    if (!haveEarthworm()){throw std::runtime_error("Recompile with earthworm");}
#ifdef WITH_EARTHWORM
    if (!isConnected()){throw std::runtime_error("Not to connected to a ring");}
    pImpl->mLogger->debug("Reading from ring...");
    // The algorithm works as follows:
    //  (1) Take the information off the ring as fast as possible.
    //  (2) Unpack the tracebuffers
    // To do (1) first attempt to allocate enough space. 
    int nWork = std::max(1024, pImpl->mMostWavesRead);
    std::vector<std::array<char, MAX_TRACEBUF_SIZ>> messageWork;
    std::vector<unsigned char> messageType;
    messageWork.reserve(nWork);
    messageType.reserve(nWork);
    pImpl->mTraceBuf2Messages.resize(0);
    // Now copy the (unpacked) messages from the ring
    std::array<char, MAX_TRACEBUF_SIZ> msg;
    MSG_LOGO gotLogo;
    long gotSize = 0;
    int returnCode = 0;
    unsigned char sequenceNumber;
    int nRead = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while(true)
    {
        // Not really sure what to with a kill signal
        returnCode = tport_getflag(&pImpl->mRegion);
        if (returnCode == TERMINATE)
        {
            auto error = "Receiving kill signal from ring: " + pImpl->mRingName
                       + "\nDisconneting from ring...";
            pImpl->mLogger->error(error);
            disconnect();
            throw std::runtime_error(error);
        }
        // Copy the ring message
        std::fill(msg.begin(), msg.end(), '\0');
        returnCode = tport_copyfrom(&pImpl->mRegion,
                                    pImpl->mLogos.data(),
                                    pImpl->mLogos.size(),
                                    &gotLogo, &gotSize,
                                    msg.data(), MAX_TRACEBUF_SIZ,
                                    &sequenceNumber);
        // Are we done?
        if (returnCode == GET_NONE){break;}
        // Handle earthworm errors
        if (returnCode != GET_OK)
        {
            if (returnCode == GET_MISS)
            {
                pImpl->mLogger->error("Some messages were missed");
            }
            else if (returnCode == GET_NOTRACK)
            {
                pImpl->mLogger->error("Message exceeded NTRACK_GET");
            }
            else if (returnCode == GET_TOOBIG)
            {
                pImpl->mLogger->error("TraceBuf2 message too big");
            }
            else if (returnCode == GET_MISS_LAPPED)
            {
                pImpl->mLogger->error("Some messages were overwritten");
            }
            else if (returnCode == GET_MISS_SEQGAP)
            {
                pImpl->mLogger->error("A gap in messages was detected");
            }
            else
            {
                pImpl->mLogger->error("Unknown earthworm error: "
                                    + std::to_string(returnCode));
            }
            continue;
        }
        // Unpack the tracebuf2 type message
        if (gotLogo.type == pImpl->mTraceBuffer2Type)
        {
            // Note, there's an optimization to be had by only copying 
            // gotSize bytes.  But for now, this is simple in terms of
            // memory (re)allocation.
            messageWork.push_back(msg);
            messageType.push_back(gotLogo.type);
        }
/*
        else if (gotLogo.type == pImpl->mTraceComp2Type)
        {
            pImpl->mLogger->error("TYPE_TRACE2_COMP_UA not handled");
        }
*/
#ifdef WITH_MSEED
        else if (gotLogo.type == pImpl->mMSEEDType)
        {
            pImpl->mLogger->error("MSEED message not handled");
            messageWork.push_back(msg);
            messageType.push_back(gotLogo.type);
        }
#endif
        else
        {
            pImpl->mLogger->error("Unhandled message type");
            continue;
        }
        nRead = nRead + 1;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration<double> (end - start).count();
    pImpl->mLogger->debug("Read " + std::to_string(nRead)
                        + " messages from ring in "
                        + std::to_string(elapsedTime) + " (s).");
    if (pImpl->mMilliSecondsWait > 0){sleep_ew(pImpl->mMilliSecondsWait);}
    // Update our typical allocation size
    pImpl->mMostWavesRead = std::max(pImpl->mMostWavesRead, 
                                     static_cast<int> (messageWork.size()));
    if (static_cast<int> (messageWork.size()) > pImpl->mMostWavesRead)
    {
        pImpl->mLogger->debug("Extending message workspace to " 
                            + std::to_string(pImpl->mMostWavesRead)
                            + " messages");
    }
    // Step 2: Unpack the messages as fast as possible
    auto nTraceBuf2Messages = std::count(messageType.begin(),
                                         messageType.end(),
                                         pImpl->mTraceBuffer2Type);
    if (nTraceBuf2Messages > 0)
    {
        start = std::chrono::high_resolution_clock::now();
        pImpl->mTraceBuf2Messages.resize(messageWork.size());
        pImpl->mLogger->debug("Unpacking " + std::to_string(nTraceBuf2Messages)
                            + " traceBuf2 messages...");
#ifdef WITH_TBB
        tbb::parallel_for(tbb::blocked_range<int> (0, messageWork.size(), 1024),
                          [&](tbb::blocked_range<int> r)
        {
            for (int it = r.begin(); it < r.end(); ++it)
            {
                if (messageType[it] == pImpl->mTraceBuffer2Type)
                {
                    try
                    {
                        pImpl->mTraceBuf2Messages[it].fromEarthworm(
                            messageWork[it].data()); 
                    }
                    catch (const std::exception &e)
                    {
                        pImpl->mLogger->error(e.what());
                        continue;
                    }
                }
            }
        });
#else
        for (int it = 0; it < static_cast<int> (messageWork.size()); ++it)
        {
            if (messageType[it] == pImpl->mTraceBuffer2Type)
            {
                //EWMessageFormat::TraceBuf2<double> tb2;
                try
                {
                    //tb2.fromEarthworm(traceBuf2Work[it].data());
                    //pImpl->mTraceBuf2Messages.at(it) = std::move(tb2);
                    pImpl->mTraceBuf2Messages[it].fromEarthworm(
                        messageWork[it].data());
                }
                catch (const std::exception &e)
                {
                    pImpl->mLogger->error(e.what());
                    continue;
                }
            }
        }
#endif
        // Evict any empty messages
        pImpl->mTraceBuf2Messages.erase(
            std::remove_if(pImpl->mTraceBuf2Messages.begin(),
                           pImpl->mTraceBuf2Messages.end(),
                           [](const TraceBuf2<double> &tb2)
                           {
                              return tb2.getNumberOfSamples() == 0;
                           }),
                           pImpl->mTraceBuf2Messages.end());
        end = std::chrono::high_resolution_clock::now();
        elapsedTime = std::chrono::duration<double> (end - start).count();
        pImpl->mLogger->debug("Successfully unpacked "
                            + std::to_string(pImpl->mTraceBuf2Messages.size())
                            + " traceBuf2 messages in "
                            + std::to_string(elapsedTime) + " (s).");
    }
#ifdef WITH_MSEED
    auto nMSEEDMessages = std::count(messageType.begin(),
                                     messageType.end(),
                                     pImpl->mMSEEDType);
    if (nMSEEDMessages > 0)
    {
        pImpl->mLogger->error(
           "Need loop to unpack MSEED messages with msr_unpack");
        for (int it = 0; it < static_cast<int> (messageWork.size()); ++it)
        {
            if (messageType[it] == pImpl->mMSEEDType)
            {
                MS3Record *msr = nullptr;
                auto size = messageWork[it].size();
                msr3_parse(messageWork[it].data(), size, &msr, 1, 0);
                msr3_free(&msr);
            }
        }
    }
#endif
#endif // End on earthworm
}

/// Flushes the wave ring
void WaveRing::flush()
{
    if (!haveEarthworm()){throw std::runtime_error("Recompile with earthworm");}
#ifdef WITH_EARTHWORM
    if (!isConnected()){throw std::runtime_error("Not to connected to a ring");}
    pImpl->mLogger->debug("Flushing ring...");
    MSG_LOGO gotLogo;
    std::array<char, MAX_TRACEBUF_SIZ> msg;
    long gotSize = 0;
    int returnCode = 0;
    unsigned char sequenceNumber;
    int nMessages = 1;
    while (true)
    {
        returnCode = tport_copyfrom(&pImpl->mRegion,
                                    pImpl->mLogos.data(),
                                    pImpl->mLogos.size(),
                                    &gotLogo, &gotSize,
                                    msg.data(), MAX_TRACEBUF_SIZ,
                                    &sequenceNumber);
        if (returnCode == GET_NONE){break;}
        nMessages = nMessages + 1;
    }
    pImpl->mLogger->debug("Flushed " + std::to_string(nMessages)
                         + " messages");
    if (pImpl->mMilliSecondsWait > 0){sleep_ew(pImpl->mMilliSecondsWait);}
#endif
    pImpl->mTraceBuf2Messages.clear();
}

/// Have earthworm?
bool WaveRing::haveEarthworm() const noexcept
{
#ifdef WITH_EARTHWORM
    return true;
#else
    return false;
#endif
}

/// Get tracebuf2 messages
std::vector<TraceBuf2<double>> 
    WaveRing::getTraceBuf2Messages() const noexcept
{
    return pImpl->mTraceBuf2Messages;
}

const UMPS::Broadcasts::Earthworm::TraceBuf2<double> *
    WaveRing::getTraceBuf2MessagesPointer() const noexcept
{
    return pImpl->mTraceBuf2Messages.data();
}

int WaveRing::getNumberOfTraceBuf2Messages() const noexcept
{
    return static_cast<int> (pImpl->mTraceBuf2Messages.size());
}
