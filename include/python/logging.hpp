#ifndef UMPS_PYTHON_LOGGING_HPP
#define UMPS_PYTHON_LOGGING_HPP  
#include <memory>
#include <string>
#include <pybind11/pybind11.h>
#include <umps/logging/stdout.hpp>
#include <umps/logging/spdlog.hpp>
#include <umps/logging/level.hpp>
namespace UMPS::Python::Messaging
{
/// @class StanardOut
/// @brief A wrapper for the standard out logger.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class StandardOut
{
public:
    /// @brief Constructor.
    explicit StandardOut(UMPS::Logging::Level level = UMPS::Logging::Level::Info);
    /// @brief Copy constructor.
    StandardOut(const StandardOut &logger);
    /// @brief Move constructor.
    StandardOut(StandardOut &&logger) noexcept;
    /// @brief Destructor.
    ~StandardOut();
    /// @brief Sets the logging level.
    void setLevel(UMPS::Logging::Level level) noexcept;
    /// @result The logging level.
    [[nodiscard]] UMPS::Logging::Level getLevel() const noexcept;
    /// @brief Issue error message.
    void error(const std::string &message);
    /// @brief Issue warning message.
    void warn(const std::string &message);
    /// @brief Issue info message.
    void info(const std::string &message);
    /// @brief Issue a debug message.
    void debug(const std::string &message);
    /// @result A shared pointer to the underlying logger.
    [[nodiscard]] std::shared_ptr<UMPS::Logging::ILog> getInstance();
    StandardOut& operator=(const StandardOut &);
    StandardOut& operator=(StandardOut &&) noexcept;
private:
    UMPS::Logging::StdOut mLogger;
};
}
#endif