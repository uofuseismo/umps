#include <string>
#include <umps/logging/stdout.hpp>
#include <umps/logging/spdlog.hpp>
#include <pybind11/pybind11.h>
#include "log.hpp"

namespace
{

class StdOut
{
public:
    StdOut(const UMPS::Logging::Level level = UMPS::Logging::Level::Info) :
        mLogger(level) //std::make_shared<UMPS::Logging::StdOut> (level))
    {
    }
    StdOut(const StdOut &logger) = default;
    StdOut(StdOut &&logger) noexcept = default;
    StdOut& operator=(const StdOut &logger) = default;
    StdOut& operator=(StdOut &&logger) noexcept = default;
    ~StdOut() = default;
    void setLevel(const UMPS::Logging::Level level) noexcept
    {
        mLogger.setLevel(level);
    }
    UMPS::Logging::Level getLevel() const noexcept
    {
        return mLogger.getLevel();
    }
    void error(const std::string &message)
    {
        mLogger.error(message);
    }
    void warn(const std::string &message)
    {
        mLogger.warn(message);
    }
    void info(const std::string &message)
    {
        mLogger.info(message);
    }
    void debug(const std::string &message)
    {
        mLogger.debug(message);
    }
    [[nodiscard]] std::shared_ptr<UMPS::Logging::ILog> getInstance()
    {
        std::shared_ptr<UMPS::Logging::ILog> result 
           =  std::make_shared<UMPS::Logging::StdOut> (mLogger);
        return result;
    }
    UMPS::Logging::StdOut mLogger;
};

/*
class SpdLog
{
public:

}; 
*/

}

void PUMPS::Logging::initializeLogging(pybind11::module &m)
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
    // Terminate request
    pybind11::class_<::StdOut> stdOut(lm, "StandardOut");
    stdOut.def(pybind11::init<> ());
    stdOut.def(pybind11::init<UMPS::Logging::Level> ());
    stdOut.doc() = R""""(
This is an UMPS logger that writes messages to standard out.  This is
typically is only useful for for the early stations of application 
development.

Properties :
    level : The logging level.  By default, this is an info-level logger. 

)"""";
    stdOut.def_property("level",
                        &StdOut::getLevel,
                        &StdOut::setLevel);
    stdOut.def("error",
               &StdOut::error,
               "Issues an error message.");
    stdOut.def("warn",
               &StdOut::warn,
               "Issues a warning message.");
    stdOut.def("info",
               &StdOut::info,
               "Issues an info message.");
    stdOut.def("debug",
               &StdOut::debug,
               "Issues a debug message.");
    //------------------------------------------------------------------------//
}
