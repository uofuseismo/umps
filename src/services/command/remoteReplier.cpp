#include <iostream>
#include <thread>
#include "umps/services/command/remoteReplier.hpp"
#include "umps/services/command/remoteReplierOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/logging/log.hpp"
#include "private/messaging/replySocket.hpp"

using namespace UMPS::Services::Command;
namespace URouterDealer = UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;

class RemoteReplier::RemoteReplierImpl : public ::ReplySocket
{
public:
    RemoteReplierImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                      std::shared_ptr<UMPS::Logging::ILog> logger) :
        ::ReplySocket(context, logger)
    {
    }
/*
    ~RemoteReplierImpl()
    {
        stop();
    }
    void stop()
    {
        if (mReplier.isRunning()){mReplier.stop();}
        if (mReplierThread.joinable()){mReplierThread.join();}
    }
    void start()
    {
        mReplierThread = std::thread(&URouterDealer::Reply::start, &mReplier);
    }
    ReplySocket mReplySocket;
    URouterDealer::Reply mReplier;
    RemoteReplierOptions mOptions;
    std::thread mReplierThread;
    bool mInitialized{false};
*/
    RemoteReplierOptions mOptions;
};

/// C'tor
RemoteReplier::RemoteReplier() :
    pImpl(std::make_unique<RemoteReplierImpl> (nullptr, nullptr))
{
}

RemoteReplier::RemoteReplier(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RemoteReplierImpl> (context, nullptr))
{
}

RemoteReplier::RemoteReplier(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteReplierImpl> (nullptr, logger))
{
}

RemoteReplier::RemoteReplier(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RemoteReplierImpl> (context, logger))
{
}

/// Destructor
RemoteReplier::~RemoteReplier() = default;

/// Initialize
void RemoteReplier::initialize(const RemoteReplierOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
    pImpl->connect(options.getOptions());
    pImpl->mOptions = options;
}

/// Initialized
bool RemoteReplier::isInitialized() const noexcept
{
    return pImpl->isConnected();
}

/// Start
void RemoteReplier::start()
{
    if (!isInitialized()){throw std::runtime_error("Replier not initialized");}
    pImpl->start();
}

/// Stop
void RemoteReplier::stop()
{
    pImpl->stop();
}

/// Connection information
UCI::SocketDetails::Reply RemoteReplier::getSocketDetails() const
{
    return pImpl->getSocketDetails();
}

