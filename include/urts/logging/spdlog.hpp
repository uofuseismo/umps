#ifndef URTS_LOGGING_SPDLOG_HPP
#define URTS_LOGGING_SPDLOG_HPP
#include <memory>
#include "urts/logging/log.hpp"
namespace URTS::Logging
{
/// SpdLog "spdlog.hpp" "urts/logging/spdlog.hpp"
/// @brief A daily logger as implemented by the spdlog library.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class SpdLog : public ILog
{
public:
    /// @brief Constructor.
    SpdLog();
    /// @brief Copy constructor.
    SpdLog(const SpdLog &spdlog);
    /// @brief Move construtor.
    SpdLog(SpdLog &&spdlog) noexcept;

    /// @brief Copy assignment operator.
    /// @param[in] spdlog   The logging class to copy to this.
    /// @result A deep copy of the spdlog class.
    SpdLog &operator=(const SpdLog &spdlog); 
    /// @brief Move assignment operator.
    /// @param[in,out] spdlog  The logging class whose memory will be moved to
    ///                        this.  On exit, spdlog's behavior is undefined.
    /// @result The memory from spdlog moved to this.
    SpdLog &operator=(SpdLog &&spdlog) noexcept;

    /// @brief Destructor.
    virtual ~SpdLog();

    /// @brief Initializes the logger.
    /// @param[in] loggerName  The logger name.  For example,
    ///                        this could be the module name.
    /// @param[in] fileName    The name of the log file.
    /// @param[in] level       The logging level.
    /// @param[in] hour        Rotate the log on this hour of the day.
    ///                        This should be in the range [0,23].
    /// @param[in] minute      Rotate the log on this minute of the hour.
    ///                        This should be in the range [0,59].
    /// @throws std::invalid_argument if the root
    void initialize(const std::string &loggerName,
                    const std::string &fileName,
                    Level level = Level::INFO,
                    int hour = 0, int minute = 0);

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
private:
    class SpdLogImpl;
    std::unique_ptr<SpdLogImpl> pImpl; 
};
}
#endif
