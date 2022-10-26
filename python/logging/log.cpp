#include <string>
#include <umps/logging/stdout.hpp>
#include <umps/logging/spdlog.hpp>
#include <pybind11/pybind11.h>
#include "python/logging.hpp"
#include "log.hpp"

using namespace UMPS::Python::Logging;

///--------------------------------------------------------------------------///
///                              StdOut Logger                               ///
///--------------------------------------------------------------------------///
/// C'tor
StandardOut::StandardOut(const UMPS::Logging::Level level) :
    mLogger(level)
{
}

/// Copy c'tor
StandardOut::StandardOut(const StandardOut &logger)
{
    *this = logger;
}

/// Move c'tor
StandardOut::StandardOut(StandardOut &&logger) noexcept
{
    *this = std::move(logger);
}

/// Destructor
StandardOut::~StandardOut() = default;

/// Copy assignment
StandardOut& StandardOut::operator=(const StandardOut &) = default;

/// Move assignment
StandardOut& StandardOut::operator=(StandardOut &&) noexcept = default;

/// Set level
void StandardOut::setLevel(const UMPS::Logging::Level level) noexcept
{
    mLogger.setLevel(level);
}

UMPS::Logging::Level StandardOut::getLevel() const noexcept
{
    return mLogger.getLevel();
}

/// Error
void StandardOut::error(const std::string &message)
{
    mLogger.error(message);
}

/// Warn
void StandardOut::warn(const std::string &message)
{
    mLogger.warn(message);
}

/// Info
void StandardOut::info(const std::string &message)
{
    mLogger.info(message);
}

/// Debug
void StandardOut::debug(const std::string &message)
{
    mLogger.debug(message);
}

/// Make instance
std::shared_ptr<UMPS::Logging::ILog> StandardOut::getInstance()
{
    std::shared_ptr<UMPS::Logging::ILog> result
       =  std::make_shared<UMPS::Logging::StdOut> (mLogger);
    return result;
}

///--------------------------------------------------------------------------///
///                                 Day Logger                               ///
///--------------------------------------------------------------------------///
Daily::Daily() = default;

/// Copy c'tor
Daily::Daily(const Daily &logger)
{
    *this = logger;
}

/// Move c'tor
Daily::Daily(Daily &&logger) noexcept
{
    *this = std::move(logger);
}

/// Destructor
Daily::~Daily() = default;

/// Copy assignment
Daily& Daily::operator=(const Daily &) = default;

/// Move assignment
Daily& Daily::operator=(Daily &&) noexcept = default;

void Daily::initialize(const std::string &loggerName,
                       const std::string &fileName,
                       const UMPS::Logging::Level level,
                       const int hour,
                       const int minute)
{
    mLogger.initialize(loggerName, fileName, level, hour, minute);
}

/// Error
void Daily::error(const std::string &message)
{
    mLogger.error(message);
}

/// Warn
void Daily::warn(const std::string &message)
{
    mLogger.warn(message);
}

/// Info
void Daily::info(const std::string &message)
{
    mLogger.info(message);
}

/// Debug
void Daily::debug(const std::string &message)
{
    mLogger.debug(message);
}

UMPS::Logging::Level Daily::getLevel() const noexcept
{
    return mLogger.getLevel();
}

/// Make instance
std::shared_ptr<UMPS::Logging::ILog> Daily::getInstance()
{
    std::shared_ptr<UMPS::Logging::ILog> result
       =  std::make_shared<UMPS::Logging::SpdLog> (mLogger);
    return result;
}

///--------------------------------------------------------------------------///
///                               Initialization                             ///
///--------------------------------------------------------------------------///
void UMPS::Python::Logging::initialize(pybind11::module &m)
{
    pybind11::module lm = m.def_submodule("Logging");
    lm.attr("__doc__") = "Logging utilities for UMPS.";
    // Enums
    pybind11::enum_<UMPS::Logging::Level> (lm, "Level")
        .value("Error",
               UMPS::Logging::Level::Error,
               "Only errors are logged.")
        .value("Warn",
               UMPS::Logging::Level::Warn,
               "Warnings and errors are logged.")
        .value("Info",
               UMPS::Logging::Level::Info,
               "Warnings, errors, and info messages are logged.")
        .value("Debug",
               UMPS::Logging::Level::Debug,
               "Everything is logged.");
    // Standard out logger
    pybind11::class_<UMPS::Python::Logging::StandardOut>
         stdOut(lm, "StandardOut");
    stdOut.def(pybind11::init<> ());
    stdOut.def(pybind11::init<UMPS::Logging::Level> ());
    stdOut.doc() = R""""(
This is an UMPS logger that writes messages to standard out.  This is
typically is only useful for for the early stations of application 
development.

Properties :
    level : int
        The logging level.  By default, this is an info-level logger. 

)"""";
    stdOut.def_property("level",
                        &StandardOut::getLevel,
                        &StandardOut::setLevel);
    stdOut.def("error",
               &StandardOut::error,
               "Issues an error message.");
    stdOut.def("warn",
               &StandardOut::warn,
               "Issues a warning message.");
    stdOut.def("info",
               &StandardOut::info,
               "Issues an info message.");
    stdOut.def("debug",
               &StandardOut::debug,
               "Issues a debug message.");
    //------------------------------------------------------------------------//
    // Day logger
    // Standard out logger
    pybind11::class_<UMPS::Python::Logging::Daily> daily(lm, "Daily");
    daily.def(pybind11::init<> ());
    daily.doc() = R""""(
This is an UMPS logger that writes messages to a file.  The file is rotated
every day on the hour/minute specified during initialization.  The log output
will look like:

     [timestamp] [Logger Name] Message

Read-only Properties :
    level : int
        The logging level.  By default, this is an info-level logger. 
)"""";
    daily.def_property_readonly("level",
                                &Daily::getLevel);
    daily.def("initialize",
              &Daily::initialize,
R""""(
Initializes the logger.\n
\n
Parameters\n
----------\n
logger_name : str\n
    The logger name.  This is how to identify the log in the output.\n
file_name : str\n
    The file to which to write messages.\n
hour : int\n
    Rotate the log on this hour of the day.  This should be in the range [0,23].\n
minute : int\n
    Rotate the log on this minute of the hour.  This should be in the range [0,59].\n
)"""");
    daily.def("error",
              &Daily::error,
              "Issues an error message.");
    daily.def("warn",
              &Daily::warn,
              "Issues a warning message.");
    daily.def("info",
              &Daily::info,
              "Issues an info message.");
    daily.def("debug",
              &Daily::debug,
              "Issues a debug message.");
}
