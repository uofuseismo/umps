#ifndef UMPS_LOGGING_LOGIT_HPP
#define UMPS_LOGGING_LOGIT_HPP
#include <memory>
#include "umps/logging/log.hpp"
namespace UMPS::Logging
{
/// @class LogIt "logIt.hpp" "umps/logging/logIt.hpp"
/// @brief A wrapper to Earthworm's logit facility.  If this library is not
///        compiled against Earthworm then all calls will be written to stdout.
/// @note This class is a stub.  As per usual, libew is a hot mess and there
///       are missing functions during linking.
class LogIt : public ILog
{
public:
    /// @name Constructor
    /// @{

    /// @brief Constructor.
    LogIt();
    /// @}

    /// @param[in] programName  The program name.  Earthworm will try to make
    ///                         log files from this and write to the EW_LOG
    ///                         environment variable directory.
    /// @param[in] level        Controls the log level.
    /// @param[in] logFlag      I have no idea.  Some earthworm thing.
    void initialize(const std::string &programName,
                    Level level = Level::INFO,
                    int logFlag = 1);

    /// @result Gets the logging level.
    [[nodiscard]] Level getLevel() const noexcept override;
    /// @brief Writes an error message.
    /// @note This requires \c getLevel() be >= Level::ERROR.
    void error(const std::string &message) override;
    /// @brief Writes a warning message.
    /// @note This requires \c getLevel() be >= Level::WARN.
    void warn(const std::string &message) override;
    /// @brief Writes an info message.
    /// @note This requires \c getLevel() be >= least Level::INFO.
    void info(const std::string &message) override;
    /// @brief Writes a debug message.
    /// @note This requires \c getLevel() be >= Level::DEBUG.
    void debug(const std::string &message) override;

    /// @brief Destructor
    /// @{

    ~LogIt() override;
    /// @}

    /// @result True indicates the library was compiled with earthworm.
    [[nodiscard]] static bool haveEarthworm() noexcept;

    LogIt(const LogIt &) = delete;
    LogIt(LogIt &&) noexcept = delete;
    LogIt &operator=(const LogIt &) = delete;
    LogIt &operator=(LogIt &&) noexcept = delete;
private:
    class LogItImpl;
    std::unique_ptr<LogItImpl> pImpl;
};
}
#endif 
