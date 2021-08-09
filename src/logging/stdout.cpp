#include <iostream>
#include "urts/logging/stdout.hpp"

using namespace URTS::Logging;

class StdOut::StdOutImpl
{
public:
    Level mLevel = Level::INFO;
};

/// C'tor
StdOut::StdOut() :
    pImpl(std::make_unique<StdOutImpl> ())
{
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
        std::cout << message << std::endl;
    }
}

/// Warn
void StdOut::warn(const std::string &message)
{
    if (pImpl->mLevel >= Level::WARN)
    {
        std::cout << message << std::endl;
    }
}

/// Error
void StdOut::error(const std::string &message)
{
    if (pImpl->mLevel >= Level::ERROR)
    {
        std::cerr << message << std::endl;
    }
}

/// Debug
void StdOut::debug(const std::string &message)
{
    if (pImpl->mLevel >= Level::DEBUG)
    {
        std::cerr << message << std::endl;
    }
}
