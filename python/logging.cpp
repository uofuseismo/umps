#include <string>
#include <umps/logging/standardOut.hpp>
#include <umps/logging/dailyFile.hpp>
#include <umps/logging/level.hpp>
#include <pybind11/pybind11.h>
#include "python/logging.hpp"

using namespace UMPS::Python::Logging;

///--------------------------------------------------------------------------///
///                              StandardOut Logger                          ///
///--------------------------------------------------------------------------///
/// Constructor
StandardOut::StandardOut(const UMPS::Logging::Level level) :
    mLogger(level)
{
}

/// Copy constructor
StandardOut::StandardOut(const StandardOut &logger)
{
    *this = logger;
}

/// Move constructor
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
std::shared_ptr<UMPS::Logging::ILog> StandardOut::getSharedPointer()
{
    std::shared_ptr<UMPS::Logging::ILog> result
       =  std::make_shared<UMPS::Logging::StandardOut> (mLogger);
    return result;
}

///--------------------------------------------------------------------------///
///                                 Day Logger                               ///
///--------------------------------------------------------------------------///
DailyFile::DailyFile() = default;

/// Copy c'tor
/*
DailyFile::DailyFile(const DailyFile &logger)
{
    *this = logger;
}
*/

/// Move c'tor
DailyFile::DailyFile(DailyFile &&logger) noexcept
{
    *this = std::move(logger);
}

/// Destructor
DailyFile::~DailyFile() = default;

/// Copy assignment
//DailyFile& DailyFile::operator=(const DailyFile &) = default;

/// Move assignment
DailyFile& DailyFile::operator=(DailyFile &&) noexcept = default;

void DailyFile::initialize(const std::string &loggerName,
                           const std::string &fileName,
                           const UMPS::Logging::Level level,
                           const int hour,
                           const int minute)
{
    mLogger.initialize(loggerName, fileName, level, hour, minute);
}

/// Error
void DailyFile::error(const std::string &message)
{
    mLogger.error(message);
}

/// Warn
void DailyFile::warn(const std::string &message)
{
    mLogger.warn(message);
}

/// Info
void DailyFile::info(const std::string &message)
{
    mLogger.info(message);
}

/// Debug
void DailyFile::debug(const std::string &message)
{
    mLogger.debug(message);
}

UMPS::Logging::Level DailyFile::getLevel() const noexcept
{
    return mLogger.getLevel();
}

/// Make instance
/*
std::shared_ptr<UMPS::Logging::ILog> DailyFile::getSharedPointer()
{
    std::shared_ptr<UMPS::Logging::ILog> result
       =  std::make_shared<UMPS::Logging::DailyFile> (mLogger);
    return result;
}
*/

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
    // Logging base class
    pybind11::class_<UMPS::Python::Logging::ILog> (lm, "ILog");
    // Standard out logger
    pybind11::class_<UMPS::Python::Logging::StandardOut,
                     UMPS::Python::Logging::ILog //std::shared_ptr<UMPS::Python::Logging::StandardOut>
                     >
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
    pybind11::class_<UMPS::Python::Logging::DailyFile,
                     UMPS::Python::Logging::ILog //std::shared_ptr<UMPS::Python::Logging::DailyFile>
                     >
        daily(lm, "DailyFile");
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
                                &DailyFile::getLevel);
    daily.def("initialize",
              &DailyFile::initialize,
R""""(
Initializes the logger.

Parameters
----------
logger_name : str
    The logger name.  This is how to identify the log in the output.
file_name : str
    The file to which to write messages.
hour : int
    Rotate the log on this hour of the day.  This should be in the range [0,23].
minute : int
    Rotate the log on this minute of the hour.  This should be in the range [0,59].
)"""",
        pybind11::arg("logger_name"),
        pybind11::arg("file_name"),
        pybind11::arg("level") = UMPS::Logging::Level::Info,
        pybind11::arg("hour") = 0,
        pybind11::arg("minute") = 0);
    daily.def("error",
              &DailyFile::error,
              "Issues an error message.");
    daily.def("warn",
              &DailyFile::warn,
              "Issues a warning message.");
    daily.def("info",
              &DailyFile::info,
              "Issues an info message.");
    daily.def("debug",
              &DailyFile::debug,
              "Issues a debug message.");
}
