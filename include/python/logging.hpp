#ifndef UMPS_PYTHON_LOGGING_HPP
#define UMPS_PYTHON_LOGGING_HPP  
#include <memory>
#include <string>
#include <pybind11/pybind11.h>
#include <umps/logging/standardOut.hpp>
#include <umps/logging/dailyFile.hpp>
#include <umps/logging/level.hpp>
namespace UMPS::Python::Logging
{
/// @class StandardOut
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
    UMPS::Logging::StandardOut mLogger;
};
/// @class DailyFile
/// @brief A logger that will dump messages for a given day to a file.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class DailyFile
{
public:
    /// @brief Constructor.
    DailyFile();
    /// @brief Copy constructor.
    DailyFile(const DailyFile &daily);
    /// @brief Move constructor.
    DailyFile(DailyFile &&daily) noexcept;
    /// @brief Initialize the log.
    void initialize(const std::string &loggerName,
                    const std::string &fileName,
                    UMPS::Logging::Level level = UMPS::Logging::Level::Info,
                    int hour = 0,
                    int minute = 0); 
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
    /// @brief Destructor.
    ~DailyFile();
    DailyFile& operator=(const DailyFile &); 
    DailyFile& operator=(DailyFile &&) noexcept;
private:
    UMPS::Logging::DailyFile mLogger;
};
void initialize(pybind11::module &m);
}
#endif
