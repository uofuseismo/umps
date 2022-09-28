#include "umps/services/command/remoteReplier.hpp"
#include "umps/services/command/remoteReplierOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/log.hpp"

using namespace UMPS::Services::Command;
namespace URouterDealer = UMPS::Messaging::RouterDealer;

class RemoteReplier::RemoteReplierImpl
{
public:
    RemoteReplierImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                      std::shared_ptr<UMPS::Logging::ILog> logger) :
        mReplier(context, logger)
    {
    }
    URouterDealer::Reply mReplier;
    RemoteReplierOptions mOptions;
    bool mInitialized{false};
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
    auto replyOptions = options.getOptions();
    pImpl->mReplier.initialize(replyOptions);
    pImpl->mOptions = options;
    pImpl->mInitialized = true;     
}

/// Initialized
bool RemoteReplier::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Start
void RemoteReplier::start()
{
    if (!isInitialized()){throw std::runtime_error("Replier not initialized");}
    pImpl->mReplier.start();
}

/// Stop
void RemoteReplier::stop()
{
    pImpl->mReplier.stop();
}
