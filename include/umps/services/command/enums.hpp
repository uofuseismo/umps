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
/// @brief The service's return code for a terminate request.
enum class TerminateReturnCode
{
    Success = 0,         /*!< Indicates the command was successful. */
    InvalidCommand = 1,  /*!< The command is invalid. */
    ApplicationError = 2 /*!< The terminate request is valid but the
                              application could not process the request. */
};
/// @brief The service's return code for a command request.
enum class CommandReturnCode
{
    Success = 0,         /*!< Indicates the command was successful. */
    InvalidCommand = 1,  /*!< Indicates the command was invalid. */
    ApplicationError = 2 /*!< The code is valid but the application threw an error. */
};
/// @brief The service's return code for a registration request.
enum class RegistrationReturnCode
{
    Success = 0,        /*!< The registration request succeeded. */
    InvalidRequest = 1, /*!< The registration rqeuest failed because the
                             request was invalid. */
    Exists = 2,         /*!< The registration request failed because the
                             module already exists. */
    ServerError = 3     /*!< The registration request failed because of a
                             server-side error. */ 
};
}
#endif