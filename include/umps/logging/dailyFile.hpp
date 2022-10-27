#ifndef UMPS_LOGGING_DAILY_FILE_HPP
#define UMPS_LOGGING_DAILY_FILE_HPP
#include <memory>
#include "umps/logging/log.hpp"
namespace UMPS::Logging
{
/// DailyFile "dailyFile.hpp" "umps/logging/dailyFile.hpp"
/// @brief A daily logger as implemented by the SpdLog library.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class DailyFile : public ILog
{
public:
    /// @name Cosntructors
    /// @{

    /// @brief Constructor.
    DailyFile();
    /// @brief Copy constructor.
    /// @param[in] logger  The logger class from which to initialize this class.
    DailyFile(const DailyFile &logger);
    /// @brief Move construtor.
    /// @param[in,out] logger  The logger class from which to initialize this
    ///                        class.  On exit, logger's behavior is undefined.
    DailyFile(DailyFile &&logger) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] logger   The logging class to copy to this.
    /// @result A deep copy of the logger class.
    DailyFile &operator=(const DailyFile &logger); 
    /// @brief Move assignment operator.
    /// @param[in,out] logger  The logging class whose memory will be moved to
    ///                        this.  On exit, logger's behavior is undefined.
    /// @result The memory from logger moved to this.
    DailyFile &operator=(DailyFile &&logger) noexcept;
    /// @}

    /// @name Initialization
    /// @{

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
                    Level level = Level::Info,
                    int hour = 0,
                    int minute = 0);
    /// @result Gets the logging level.
    [[nodiscard]] Level getLevel() const noexcept override;
    /// @}

    /// @name Issue A Log Message 
    /// @{

    /// @brief Writes an error message.
    /// @note This requires \c getLevel() be >= Level::Error.
    void error(const std::string &message) override;
    /// @brief Writes a warning message.
    /// @note This requires \c getLevel() be >= Level::Warn.
    void warn(const std::string &message) override;
    /// @brief Writes an info message.
    /// @note This requires \c getLevel() be >= least Level::Info.
    void info(const std::string &message) override;
    /// @brief Writes a debug message.
    /// @note This requires \c getLevel() be >= Level::Debug.
    void debug(const std::string &message) override;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~DailyFile() override;
    /// @}
private:
    class DailyFileImpl;
    std::unique_ptr<DailyFileImpl> pImpl; 
};
}
#endif
