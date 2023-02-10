#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <string>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "umps/logging/dailyFile.hpp"

namespace fs = std::filesystem;

using namespace UMPS::Logging;

namespace
{

/// Converts the UMPS logging level to something spdlog understands.
spdlog::level::level_enum levelToLevel(const Level level)
{
    if (level == Level::Error)
    {
        return spdlog::level::err;
    }
    else if (level == Level::Warn)
    {
        return spdlog::level::warn;
    }
    else if (level == Level::Info)
    {
        return spdlog::level::info;
    }
    else if (level == Level::Debug)
    {
        return spdlog::level::debug;
    }
    return spdlog::level::off;
}

}

/// Implementation
class DailyFile::DailyFileImpl
{
public:
/*
    ~DailFileImpl()
    {
        stop();
    }
    void stop()
    {
        setRunning(false);
        if (mFlushingThread.joinable()){mFlushingThread.join();}
    }
    void start()
    {
        stop();
    }
    void periodicallyFlush( )
    {
        while (keepRunning())
        {
            std::unique_lock<std::mutex> lock(mMutex);
            dataCondition.wait(lock, []{return 
            lock.unlock();
        }
    }
    void setRunning(const bool keepRunning) noexcept
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mKeepRunning = keepRunning;
    }
*/
    std::shared_ptr<spdlog::logger> mLogger{nullptr};
    Level mLevel{Level::INFO};
    //bool mKeepRunning{false};
};

/// C'tor
DailyFile::DailyFile() :
    pImpl(std::make_unique<DailyFileImpl> ())
{
}

/// Copy c'tor
DailyFile::DailyFile(const DailyFile &logger)
{
    *this = logger;
}

/// Move c'tor
DailyFile::DailyFile(DailyFile &&logger) noexcept
{
    *this = std::move(logger);
}

/// Copy assignment
DailyFile& DailyFile::operator=(const DailyFile &logger)
{
    if (&logger == this){return *this;}
    pImpl = std::make_unique<DailyFileImpl> (*logger.pImpl);
    return *this;
}

/// Move assignment
DailyFile& DailyFile::operator=(DailyFile &&logger) noexcept
{
    if (&logger == this){return *this;}
    pImpl = std::move(logger.pImpl);
    return *this;
}

/// Destructor
DailyFile::~DailyFile() = default;

/// Get the logging level
Level DailyFile::getLevel() const noexcept
{
    return pImpl->mLevel;
}

/// Initialize the logger
void DailyFile::initialize(const std::string &loggerName,
                           const std::string &fileName,
                           const Level level,
                           const int hour, const int minute)
{
    // Check the inputs
    if (hour < 0 || hour > 23)
    {
        auto errmsg = "hour = " + std::to_string(hour)
                    + " must be in range [0,23]";
        spdlog::critical(errmsg);
        throw std::invalid_argument(errmsg);
    }
    if (minute < 0 || minute > 59)
    {
        auto errmsg = "minute = " + std::to_string(minute)
                    + " must be in range [0,59]";
        spdlog::critical(errmsg);
        throw std::invalid_argument(errmsg);
    }
    if (loggerName.empty())
    {
        auto errmsg = "Logger name is empty";
        spdlog::critical(errmsg);
        throw std::invalid_argument(errmsg);
    }
    if (fileName.empty())
    {
        auto errmsg = "Filename is empty";
        spdlog::critical(errmsg);
        throw std::invalid_argument(errmsg);
    }
    // Attempt to make a logging directory 
    auto path = fs::path(fileName);
    if (!fs::exists(path))
    {
        auto parentPath = path.parent_path();
        if (!parentPath.empty())
        {
            if (!fs::exists(parentPath))
            {
                if (!fs::create_directories(parentPath))
                {
                    auto errmsg = "Could not create logging directory: "
                                + parentPath.string();
                    spdlog::critical(errmsg);
                    throw std::runtime_error(errmsg);
                }
            }
        }
    }
    // Initialize spdlog logger
    constexpr bool truncate = false;
    constexpr int maxFiles = 0;
    auto spdlogLevel = levelToLevel(level);
    pImpl->mLevel = level;
    pImpl->mLogger = spdlog::daily_logger_mt(loggerName,
                                             fileName,
                                             hour,
                                             minute,
                                             truncate,
                                             maxFiles);
    pImpl->mLogger->set_level(spdlogLevel);
}

/// Info
void DailyFile::info(const std::string &message)
{
    if (pImpl->mLevel >= Level::Info && pImpl->mLogger)
    {
        //spdlog::info(message);
        pImpl->mLogger->info(message);
        pImpl->mLogger->flush();
    }
}

/// Warn
void DailyFile::warn(const std::string &message)
{
    if (pImpl->mLevel >= Level::Warn && pImpl->mLogger)
    {
        //spdlog::warn(message);
        pImpl->mLogger->warn(message);
        pImpl->mLogger->flush();
    }
}

/// Error
void DailyFile::error(const std::string &message)
{
    if (pImpl->mLevel >= Level::Error && pImpl->mLogger)
    {
        //spdlog::error(message);
        pImpl->mLogger->error(message);
        pImpl->mLogger->flush();
    }
}

/// Debug
void DailyFile::debug(const std::string &message)
{
    if (pImpl->mLevel >= Level::Debug && pImpl->mLogger)
    {
        //spdlog::debug(message);
        pImpl->mLogger->debug(message);
        pImpl->mLogger->flush();
    }
}  
