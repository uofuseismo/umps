#include <string>
#include "umps/proxyServices/command/replierProcess.hpp"
#include "umps/proxyServices/command/replier.hpp"
#include "umps/proxyServices/command/replierOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/standardOut.hpp"

using namespace UMPS::ProxyServices::Command;

class ReplierProcess::ReplierProcessImpl
{
public:
    ReplierProcessImpl(
            std::shared_ptr<UMPS::Messaging::Context> context,
            const std::shared_ptr<UMPS::Logging::ILog> &logger) :
            mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StandardOut> ();
        }
        mReplier = std::make_unique<Replier> (context, mLogger);
    }
    /// Destructor
    ~ReplierProcessImpl()
    {
        stop();
    }
    /// Stop the replier process
    void stop()
    {
        if (mReplier != nullptr){mReplier->stop();}
    }
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<Replier> mReplier{nullptr};
    std::string mModuleName{"ModuleRegistryReplier"};
    bool mInitialized{false};
};

/// Constructor
ReplierProcess::ReplierProcess() :
    pImpl(std::make_unique<ReplierProcessImpl> (nullptr, nullptr))
{
}

/// Constructor
ReplierProcess::ReplierProcess(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierProcessImpl> (nullptr,logger))
{
}

/// Constructor
ReplierProcess::ReplierProcess(
        std::shared_ptr<UMPS::Messaging::Context> &context) :
        pImpl(std::make_unique<ReplierProcessImpl> (context, nullptr))
{
}

/// Destructor
ReplierProcess::~ReplierProcess() = default;

/// Initialized?
bool ReplierProcess::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Name
std::string ReplierProcess::getName() const noexcept
{
    return pImpl->mModuleName;
}

/// Stop
void ReplierProcess::stop()
{
    pImpl->stop();
}
