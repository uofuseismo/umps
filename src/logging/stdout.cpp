#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "umps/logging/stdout.hpp"

using namespace UMPS::Logging;

class StdOut::StdOutImpl
{
public:
    StdOutImpl() :
        mStdOutSink(std::make_shared<spdlog::sinks::stdout_color_sink_mt> ()),
        mLogger(std::make_shared<spdlog::logger> ("stdout", mStdOutSink))
    {
//        mLogger = spdlog::create("stdout", mStdOutSink);
    }
    //StdOutImpl() :
    //    mSink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>() )
    //{   
    //}
    //std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> mSink;
    Level mLevel = Level::Info;
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> mStdOutSink;// = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    std::shared_ptr<spdlog::logger> mLogger = nullptr;
};

/// C'tor
StdOut::StdOut(const Level level) :
    pImpl(std::make_unique<StdOutImpl> ())
{
    setLevel(level);
}

/// Copy c'tor
StdOut::StdOut(const StdOut &logger)
{
    *this = logger;
}

/// Move c'tor
StdOut::StdOut(StdOut &&logger) noexcept
{
    *this = std::move(logger);
}

/// Copy assignment
StdOut& StdOut::operator=(const StdOut &logger)
{
    if (&logger == this){return *this;}
    pImpl = std::make_unique<StdOutImpl> (*logger.pImpl);
    return *this;
}

/// Move assignment
StdOut& StdOut::operator=(StdOut &&logger) noexcept
{
    if (&logger == this){return *this;}
    pImpl = std::move(logger.pImpl);
    return *this;
}

/// Destructor
StdOut::~StdOut() = default;

/// Level
void StdOut::setLevel(const Level level) noexcept
{
    pImpl->mLevel = level;
    if (level == Level::Error)
    {
        pImpl->mLogger->set_level(spdlog::level::err);
    }
    else if (level == Level::Warn)
    {
        pImpl->mLogger->set_level(spdlog::level::warn);
    }
    else if (level == Level::Info)
    {
        pImpl->mLogger->set_level(spdlog::level::info);
    }
    else if (level == Level::Debug)
    {
        pImpl->mLogger->set_level(spdlog::level::debug);
    }
    else
    {
        pImpl->mLogger->set_level(spdlog::level::off);
    }
}

Level StdOut::getLevel() const noexcept
{   
    return pImpl->mLevel;
}

/// Info
void StdOut::info(const std::string &message)
{
    if (pImpl->mLevel >= Level::Info)
    {
        //std::cout << message << std::endl;
        //spdlog::info(message);
        pImpl->mLogger->info(message);
    }
}

/// Warn
void StdOut::warn(const std::string &message)
{
    if (pImpl->mLevel >= Level::Warn)
    {
        //std::cout << message << std::endl;
        //spdlog::warn(message);
        pImpl->mLogger->warn(message);
    }
}

/// Error
void StdOut::error(const std::string &message)
{
    if (pImpl->mLevel >= Level::Error)
    {
        //std::cerr << message << std::endl;
        //spdlog::error(message);
        pImpl->mLogger->error(message);
    }
}

/// Debug
void StdOut::debug(const std::string &message)
{
    if (pImpl->mLevel >= Level::Debug)
    {
        //std::cerr << message << std::endl;
        //spdlog::debug(message);
        pImpl->mLogger->debug(message);
    }
}
