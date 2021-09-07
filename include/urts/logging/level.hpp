#ifndef URTS_LOGGING_LEVEL_HPP
#define URTS_LOGGING_LEVEL_HPP
#include <cstdint>
namespace URTS::Logging
{
/// @class Level "level.hpp" "urts/logging/level.hpp"
/// @brief Defines the logging level.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class Level : uint32_t
{
    NONE  = 0, /*!< This will not log anything. */
    ERROR = 1, /*!< This will log only error messages. */
    WARN =  2, /*!< This will log warning and error messages. */
    INFO =  3, /*!< This will log errors, warnings, and info messages. */
    DEBUG = 4  /*!< This will log errors, warnings, info, and debugging
                     messages. */
};
}
#endif