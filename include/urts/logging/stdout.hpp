#ifndef URTS_LOGGING_STDOUT_HPP
#define URTS_LOGGING_STDOUT_HPP
#include <memory>
#include "urts/logging/log.hpp"
namespace URTS::Logging
{
/// StdOut "stdout.hpp" "urts/logging/stdout.hpp"
/// @brief A naive logger that prints all messages to standard out and,
///        in the case of errors, standard error.
/// @note This class is not recommended for typical use.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class StdOut : public ILog
{
public:
    /// @brief Constructor.
    StdOut();
    /// @brief Copy constructor.
    StdOut(const StdOut &logger);
    /// @brief Move construtor.
    StdOut(StdOut &&logger) noexcept;

    /// @brief Copy assignment operator.
    /// @param[in] logger   The logging class to copy to this.
    /// @result A deep copy of the logger class.
    StdOut &operator=(const StdOut &logger); 
    /// @brief Move assignment operator.
    /// @param[in,out] logger  The logging class whose memory will be moved to
    ///                        this.  On exit, logger's behavior is undefined.
    /// @result The memory from logger moved to this.
    StdOut &operator=(StdOut &&logger) noexcept;

    /// @brief Destructor.
    virtual ~StdOut();

    /// @brief Sets the logging level.
    /// @param[in] level  The logging level.
    void setLevel(Level level) noexcept;
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
    class StdOutImpl;
    std::unique_ptr<StdOutImpl> pImpl; 
};
}
#endif
