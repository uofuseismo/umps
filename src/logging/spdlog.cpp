#include <iostream>
#include <string>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "urts/logging/spdlog.hpp"

namespace fs = std::filesystem;

using namespace URTS::Logging;

namespace
{

/// Converts the URTS logging level to something spdlog understands.
spdlog::level::level_enum levelToLevel(const Level level)
{
    if (level == Level::ERROR)
    {
        return spdlog::level::err;
    }
    else if (level == Level::WARN)
    {
        return spdlog::level::warn;
    }
    else if (level == Level::INFO)
    {
        return spdlog::level::info;
    }
    else if (level == Level::DEBUG)
    {
        return spdlog::level::debug;
    }
    return spdlog::level::off;
}

};

/// Implementation
class SpdLog::SpdLogImpl
{
public:
    std::shared_ptr<spdlog::logger> mLogger = nullptr;
    Level mLevel = Level::INFO;
};

/// C'tor
SpdLog::SpdLog() :
    pImpl(std::make_unique<SpdLogImpl> ())
{
}

/// Copy c'tor
SpdLog::SpdLog(const SpdLog &spdlog)
{
    *this = spdlog;
}

/// Move c'tor
SpdLog::SpdLog(SpdLog &&spdlog) noexcept
{
    *this = std::move(spdlog);
}

/// Copy assignment
SpdLog& SpdLog::operator=(const SpdLog &spdlog)
{
    if (&spdlog == this){return *this;}
    pImpl = std::make_unique<SpdLogImpl> (*spdlog.pImpl);
    return *this;
}

/// Move assignment
SpdLog& SpdLog::operator=(SpdLog &&spdlog) noexcept
{
    if (&spdlog == this){return *this;}
    pImpl = std::move(spdlog.pImpl);
    return *this;
}

/// Destructor
SpdLog::~SpdLog() = default;

/// Get the logging level
Level SpdLog::getLevel() const noexcept
{
    return pImpl->mLevel;
}

/// Initialize the logger
void SpdLog::initialize(const std::string &loggerName,
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
void SpdLog::info(const std::string &message)
{
    if (pImpl->mLevel >= Level::INFO && pImpl->mLogger)
    {
        spdlog::info(message);
    }
}

/// Warn
void SpdLog::warn(const std::string &message)
{
    if (pImpl->mLevel >= Level::WARN && pImpl->mLogger)
    {
        spdlog::warn(message);
    }
}

/// Error
void SpdLog::error(const std::string &message)
{
    if (pImpl->mLevel >= Level::ERROR && pImpl->mLogger)
    {
        spdlog::error(message);
    }
}

/// Debug
void SpdLog::debug(const std::string &message)
{
    if (pImpl->mLevel >= Level::DEBUG && pImpl->mLogger)
    {
        spdlog::debug(message);
    }
}  
