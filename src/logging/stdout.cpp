#include <iostream>
#include <spdlog/spdlog.h>
//#include <spdlog/sinks/stdout_color_sinks.h>
#include "urts/logging/stdout.hpp"

using namespace URTS::Logging;

class StdOut::StdOutImpl
{
public:
    //StdOutImpl() :
    //    mSink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>() )
    //{   
    //}
    //std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> mSink;
    Level mLevel = Level::INFO;
};

/// C'tor
StdOut::StdOut() :
    pImpl(std::make_unique<StdOutImpl> ())
{
    spdlog::set_level(spdlog::level::info);
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
    if (level == Level::ERROR)
    {
        spdlog::set_level(spdlog::level::err);
    }
    else if (level == Level::DEBUG)
    {
        spdlog::set_level(spdlog::level::debug);
    }
    else if (level == Level::INFO)
    {
        spdlog::set_level(spdlog::level::info);
    }
    else if (level == Level::DEBUG)
    {
        spdlog::set_level(spdlog::level::debug);
    }
    else
    {
        spdlog::set_level(spdlog::level::off);
    }
}

Level StdOut::getLevel() const noexcept
{   
    return pImpl->mLevel;
}

/// Info
void StdOut::info(const std::string &message)
{
    if (pImpl->mLevel >= Level::INFO)
    {
        //std::cout << message << std::endl;
        spdlog::info(message);
    }
}

/// Warn
void StdOut::warn(const std::string &message)
{
    if (pImpl->mLevel >= Level::WARN)
    {
        //std::cout << message << std::endl;
        spdlog::warn(message);
    }
}

/// Error
void StdOut::error(const std::string &message)
{
    if (pImpl->mLevel >= Level::ERROR)
    {
        //std::cerr << message << std::endl;
        spdlog::error(message);
    }
}

/// Debug
void StdOut::debug(const std::string &message)
{
    if (pImpl->mLevel >= Level::DEBUG)
    {
        //std::cerr << message << std::endl;
        spdlog::debug(message);
    }
}
