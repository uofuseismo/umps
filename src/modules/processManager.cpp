#include <mutex>
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
    ProcessManagerImpl(std::shared_ptr<UMPS::Logging::ILog> logger)
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
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mLogger->debug("Adding process: " + process->getName());
        mProcesses.insert(std::pair<std::string, std::unique_ptr<IProcess>>
             {name, std::move(process)});
         
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::map<std::string, std::unique_ptr<IProcess>> mProcesses;
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
