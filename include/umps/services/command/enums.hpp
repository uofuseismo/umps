#ifndef UMPS_SERVICES_COMMAND_ENUMS_HPP
#define UMPS_SERVICES_COMMAND_ENUMS_HPP
#include <memory>
namespace UMPS::Services::Command
{
/// @brief Defines the application's status.
enum class ApplicationStatus : int 
{
    Running = 0,     /*!< This indicates the application is running. */
    NotRunning = 1,  /*!< This indicates the application is not running. */
    Unknown = 2      /*!< This indicates the application's status is unknown. */
};
/// @brief The service's return code for a command request.
enum class CommandReturnCode
{
    Success = 0,         /*!< Indicates the command was successful. */
    InvalidCommand = 1,  /*!< Indicates the command was invalid. */
    ApplicationError = 2 /*!< The code is valid but the application threw an error. */
};
}
#endif
