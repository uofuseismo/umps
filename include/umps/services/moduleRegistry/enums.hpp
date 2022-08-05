#ifndef UMPS_SERVICES_MODULEREGISTRY_ENUMS_HPP
#define UMPS_SERVICES_MODULEREGISTRY_ENUMS_HPP
namespace UMPS::Services::ModuleRegistry
{
/// @class RegistrationReturnCode enums.hpp "umps/services/moduleRegistry/enums.hpp"
/// @brief Defines return codes for a module registration request. 
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class RegistrationReturnCode
{
    Success = 0,      /*!< The module was successfully registered. */
    InvalidMessage,   /*!< The request message could not be parsed. */
    Exists,           /*!< The module was not register because an identically
                           named module already exists. */
    AlgorithmFailure  /*!< The module was not registered because of a
                           server-side error. */
};
}
#endif
