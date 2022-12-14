#include <iostream>
#include <string>
#include <filesystem>
#ifndef NDEBUG
#include <cassert>
#endif
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/proxyServices/command/replierProcess.hpp"
#include "umps/proxyServices/command/replier.hpp"
#include "umps/proxyServices/command/replierOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/standardOut.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UCI = UMPS::Services::ConnectionInformation;

#define PROCESS_NAME "ModuleRegistryReplier"


class ReplierProcess::ReplierProcessImpl
{
public:
    ReplierProcessImpl(
            const std::shared_ptr<UMPS::Messaging::Context> &context,
            const std::shared_ptr<UMPS::Logging::ILog> &logger) :
            mContext(context),
            mLogger(logger)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> ();
        }
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StandardOut>();
        }
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
    /// Start the replier
    void start()
    {
        stop(); // Stop the process
        if (mReplier == nullptr)
        {
            throw std::runtime_error("Replier not yet created");
        }
        if (!mReplier->isInitialized())
        {
            throw std::runtime_error("Replier not initialized");
        }
        mReplier->start();
    }
    /// Initialized?
    [[nodiscard]] bool isInitialized() const noexcept
    {
        if (mReplier == nullptr){return false;}
        return (mReplier->isInitialized() && mInitialized);
    }
    /// Running?
    [[nodiscard]] bool isRunning() const noexcept
    {
        if (mReplier == nullptr){return false;}
        return mReplier->isRunning();
    }
//private:
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<Replier> mReplier{nullptr};
    std::string mModuleName{PROCESS_NAME};
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

/// Constructor
ReplierProcess::ReplierProcess(
        std::shared_ptr<UMPS::Messaging::Context> &context,
        std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierProcessImpl> (context, logger))
{
}

/// Destructor
ReplierProcess::~ReplierProcess() = default;

/// Initialized?
bool ReplierProcess::isInitialized() const noexcept
{
    return pImpl->isInitialized();
}

/// Initialize the replier
void ReplierProcess::initialize(const ReplierOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::runtime_error("Address not set");
    }
    if (!options.haveCallback())
    {
        throw std::runtime_error("Callback not set");
    }
    if (!options.haveModuleDetails())
    {
        throw std::runtime_error("Module details not set");
    }
    stop(); // Stop the replier in case it is running
    // Create the replier
    pImpl->mReplier
        = std::make_unique<Replier> (pImpl->mContext, pImpl->mLogger);
    pImpl->mReplier->initialize(options);
    // All done
#ifndef NDEBUG
    assert(pImpl->mReplier->isInitialized());
#endif
    pImpl->mInitialized = true;
}

/// Start
void ReplierProcess::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Replier process not initialized");
    }
    pImpl->start();
}

/// Running
bool ReplierProcess::isRunning() const noexcept
{
    return pImpl->isRunning();
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

///--------------------------------------------------------------------------///
///                       Create From an Initialization File                 ///
///--------------------------------------------------------------------------///
[[maybe_unused]]
std::unique_ptr<ReplierProcess>
UMPS::ProxyServices::Command::createReplierProcess(
        const UCI::Requestor &requestor,
        const ModuleDetails &moduleDetails,
        const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                (const std::string &, const void *, size_t )> &callback,
        const std::string &iniFile,
        const std::string &section,
        std::shared_ptr<UMPS::Messaging::Context> context,
        std::shared_ptr<UMPS::Logging::ILog> logger)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file "
                                  + iniFile + " does not exist");
    }
    if (::isEmpty(section))
    {
        throw std::invalid_argument("Initialization file section not specified");
    }
    //
    std::string service = "ModuleRegistry";
    ReplierOptions replierOptions;
    replierOptions.setModuleDetails(moduleDetails);
    auto interval = replierOptions.getPollingTimeOut();
    auto sendHighWaterMark = replierOptions.getSendHighWaterMark();
    auto receiveHighWaterMark= replierOptions.getReceiveHighWaterMark();
    std::string address;
    // Load things from the initialization file if possible
    if (std::filesystem::exists(iniFile))
    {
        boost::property_tree::ptree propertyTree;
        boost::property_tree::ini_parser::read_ini(iniFile,
                                                   propertyTree);
        // Get the address (if applicable)
        address = propertyTree.get<std::string> (section + ".address", address);
        // Get service name
        service = propertyTree.get<std::string> (section + ".proxyService",
                                                 service);
        if (address.empty() && service.empty())
        {
            throw std::runtime_error("Must either set service name or address");
        }
        // Polling interval
        auto iInterval = static_cast<int> (interval.count());
        iInterval = propertyTree.get<int> (section + ".pollerTimeOut",
                                           iInterval);
        interval = std::chrono::milliseconds {iInterval};
        replierOptions.setPollingTimeOut(interval);
        // HWM
        receiveHighWaterMark
            = propertyTree.get<int> (section + ".receiveHighWaterMark",
                                     receiveHighWaterMark);
        replierOptions.setReceiveHighWaterMark(receiveHighWaterMark);
        sendHighWaterMark
                = propertyTree.get<int> (section + ".sendHighWaterMark",
                                         sendHighWaterMark);
        replierOptions.setSendHighWaterMark(sendHighWaterMark);
    } // End check on ini file
    // Get the service's address and the ZAP options
    if (address.empty())
    {
        address
            = requestor.getProxyServiceBackendDetails(service).getAddress();
    }
    auto zapOptions = requestor.getZAPOptions();
    replierOptions.setAddress(address);
    replierOptions.setZAPOptions(zapOptions);
    // Set the callback
    replierOptions.setCallback(callback);
    // Create
    auto result = std::make_unique<ReplierProcess> (context, logger);
    result->initialize(replierOptions);
    return result;
}
