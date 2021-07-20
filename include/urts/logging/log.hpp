#ifndef URTS_LOGGING_LOG_HPP
#define URTS_LOGGING_LOG_HPP
#include "urts/logging/level.hpp"
namespace URTS::Logging
{
/// @class ILog "log.hpp" "urts/logging/log.hpp"
/// @brief This is an abstract base class that allows users to define custom
///        loggers to be used in other applications.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license. 
class ILog
{
public:
    /// @result The logging level.
    [[nodiscard]] virtual Level getLevel() const noexcept = 0;
    /// @brief Writes an error message.
    virtual void error(const std::string &error) = 0;
    /// @brief Writes a warning message. 
    virtual void warn(const std::string &message) = 0;
    /// @brief Writes an info message.
    virtual void info(const std::string &message) = 0;
    /// @brief Writes a debug message.
    virtual void debug(const std::string &message) = 0;
};
}
#endif
