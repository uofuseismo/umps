#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include "umps/modules/processManager.hpp"
#include "umps/modules/process.hpp"
#include "umps/logging/log.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Modules;

class ProcessManager::ProcessManagerImpl
{
public:
    /// C'tor
    explicit ProcessManagerImpl(
        const std::shared_ptr<UMPS::Logging::ILog> &logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
    }
    /// Running?
    [[nodiscard]] bool isRunning() const noexcept
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mRunning;
    }
    /// Map contains given process
    [[nodiscard]] bool contains(const std::string &name) const noexcept
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mProcesses.contains(name);
    }
    /// Start processes
    void start()
    {
        stop(); // Stop processes before starting
        std::lock_guard<std::mutex> lockGuard(mMutex);
        for (auto &m : mProcesses)
        {
            mLogger->debug("Starting process: " + m.second->getName());
            try
            {
                m.second->start();
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed to start " + m.second->getName()
                             + ".  Failed with: " + e.what());
            }
        }
        mRunning = true;
    } 
    /// Stop processes
    void stop()
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        for (auto &m : mProcesses)
        {
            mLogger->debug("Stopping process: " + m.second->getName());
            try
            {
                m.second->stop();
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed to stop " + m.second->getName()
                             + ".  Failed with: " + e.what());
            }
        }
        mRunning = false;
        mStopRequested = false;
    }
    /// Insert a process
    void insert(std::unique_ptr<IProcess> &&process)
    {
        auto name = process->getName();
        if (contains(name))
        {
            throw std::invalid_argument("Process: " + name + " already exists");
        }
        process->stop();
        if (isRunning()){process->start();} // Adding while hot
        process->setStopCallback(
            [this] { issueStopNotification(); });
        mLogger->debug("Adding process: " + process->getName());
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mProcesses.insert(std::pair<std::string, std::unique_ptr<IProcess>>
             {name, std::move(process)});
    }
    /// Issues a stop notification 
    void issueStopNotification()
    {
        mLogger->debug("Issuing stop notification...");
        {
            std::lock_guard<std::mutex> lock(mStopContext);
            mStopRequested = true;
        }
        mStopCondition.notify_one();
    }
    /// Place for the main thread to sleep until someone wakes it up.
    void handleMainThread()
    {
        {
            std::unique_lock<std::mutex> lock(mStopContext);
            mStopCondition.wait(lock,
                                [this]
                                {
                                    return mStopRequested;
                                });
            lock.unlock();
        }
        if (mStopRequested)
        {
            mLogger->debug("Stop request received.  Terminating...");
            stop();
        }
    }
///private:
    mutable std::mutex mMutex;
    mutable std::mutex mStopContext;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::map<std::string, std::unique_ptr<IProcess>> mProcesses;
    std::condition_variable mStopCondition;
    bool mStopRequested{false};
    bool mRunning{false};
};

/// C'tor
ProcessManager::ProcessManager() :
    pImpl(std::make_unique<ProcessManagerImpl> (nullptr))
{
}

ProcessManager::ProcessManager(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ProcessManagerImpl> (logger))
{
}

/// Destructor
ProcessManager::~ProcessManager() = default;

/// Insert
void ProcessManager::insert(std::unique_ptr<IProcess> &&process)
{
    pImpl->insert(std::move(process));
}

/// Start the modules
void ProcessManager::start()
{
    pImpl->start();
}

/// Make main thread wait until quit command received
void ProcessManager::handleMainThread()
{
    pImpl->handleMainThread();
}

/// Stop the modules
void ProcessManager::stop()
{
    pImpl->stop();
}

/// Process exists?
bool ProcessManager::contains(const IProcess &process) const noexcept
{
    return contains(process.getName()); 
}

bool ProcessManager::contains(const std::string &name) const noexcept
{
    return pImpl->contains(name);
}

//// Running?
bool ProcessManager::isRunning() const noexcept
{
    return pImpl->isRunning();
}
