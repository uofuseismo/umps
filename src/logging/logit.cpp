#include <iostream>
#include <cstring>
#include <cstdio>
#include <earthworm_simple_funcs.h>
#include "umps/logging/logIt.hpp"
/*
#if WITH_EARTHWORM
 #define HAVE_EARTHWORM 1
#else
 #undef HAVE_EARTHWORM
#endif
*/
#undef HAVE_EARTHWORM

using namespace UMPS::Logging;

/// Implementation
class LogIt::LogItImpl
{
public:
    Level mLevel = Level::INFO;
    std::string mProgramName;
};

/// C'tor
LogIt::LogIt() :
    pImpl(std::make_unique<LogItImpl> ())
{
}

/// Destructor
LogIt::~LogIt() = default;

/// Initializes the logger
void LogIt::initialize(const std::string &programName,
                       const Level level,
#ifdef HAVE_EARTHWORM
                       const int logFlag
#else
                       const int
#endif
                       )
{
    if (programName.empty())
    {
        throw std::invalid_argument("Program name is empty");
    }
    pImpl->mLevel = level;
#ifdef HAVE_EARTHWORM
        logit_init(programName.c_str(), 0, 512, 1);
#else
        std::cerr << "UMPS not compiled with Earthworm.  "
                  << "This will send all messages to stdout." << std::endl;
#endif
}

Level LogIt::getLevel() const noexcept
{
    return pImpl->mLevel;
}

/// Have earthworm?
bool LogIt::haveEarthworm() noexcept
{
#ifdef HAVE_EARTHWORM
    return true;
#else
    return false;
#endif
}

/// Error
void LogIt::error(const std::string &message)
{
    if (pImpl->mLevel >= Level::ERROR)
    {
#ifdef HAVE_EARTHWORM
        logit("et", "%s\n", message.c_str());
#else
        std::cerr << message << std::endl;
#endif
    }
}

/// Warn
void LogIt::warn(const std::string &message)
{
    if (pImpl->mLevel >= Level::WARN)
    {
#ifdef HAVE_EARTHWORM
        logit("et", "%s\n", message.c_str());
#else
        std::cerr << message << std::endl;
#endif
    }
}

/// Info
void LogIt::info(const std::string &message)
{
    if (pImpl->mLevel >= Level::INFO)
    {
#ifdef HAVE_EARTHWORM
        logit("t", "%s\n", message.c_str());
#else
        std::cout << message << std::endl;
#endif
    }   
}

/// Debug
void LogIt::debug(const std::string &message)
{
    if (pImpl->mLevel >= Level::DEBUG)
    {
#ifdef HAVE_EARTHWORM
       logit("t", "%s\n", message.c_str());
#else
       std::cout << message << std::endl;
#endif
    }
}

