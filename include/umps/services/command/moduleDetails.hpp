#ifndef UMPS_SERVICES_COMMAND_MODULE_DETAILS_HPP
#define UMPS_SERVICES_COMMAND_MODULE_DETAILS_HPP
#include <memory>
#include <ostream>
namespace UMPS::Services::Command
{
/// @class ModuleDetails moduleDetails.hpp "umps/services/command/moduleDetails.hpp"
/// @brief Defines the module properties.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class ModuleDetails
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ModuleDetails();
    /// @brief Copy constructor.
    /// @param[in] details  The details from which to initialize this class.
    ModuleDetails(const ModuleDetails &details);
    /// @brief Move constructor.
    /// @param[in,out] details  The details from which to initialize this class.
    ///                         On exit, details's behavior is undefined.
    ModuleDetails(ModuleDetails &&details) noexcept;
    /// @}

    /// @name Required Properties
    /// @{

    /// @brief Sets the module name.
    /// @param[in] name   The module name.
    /// @throws std::invalid_argument if the name is empty.
    void setName(const std::string &name);
    /// @result The module name.
    /// @throws std::runtime_error if the module name was not set.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates the module name was set.
    [[nodiscard]] bool haveName() const noexcept;
    /// @}

    /// @name Optional Properties
    /// @{

    /// @brief Sets the executable name.  This is because different
    ///        executables can run different instances of a module.  
    /// @param[in] name  The executable name.
    void setExecutableName(const std::string &name) noexcept;
    /// @result The executable name.
    /// @note This can be blank.
    [[nodiscard]] std::string getExecutableName() const noexcept;

    /// @brief Sets the machine on which the module is running.
    /// @param[in] machine  The machine name.
    void setMachine(const std::string &machine) noexcept;
    /// @result The machine on which the module is running.
    /// @note This can be blank.
    [[nodiscard]] std::string getMachine() const noexcept;

    /// @brief Sets the identifier of the process running this module.
    /// @param[in] identifier  The process identifier.
    void setProcessIdentifier(int64_t identifier) noexcept;
    /// @result The identifier of the process running this module.
    [[nodiscard]] int64_t getProcessIdentifier() const noexcept;

    /// @brief Sets the identifier of the parent process of the process running
    ///        this module.
    /// @param[in] identifier  The process identifier.
    void setParentProcessIdentifier(int64_t identifier) noexcept;
    /// @result The identifier of the parent process of the process 
    ///         running this module.
    [[nodiscard]] int64_t getParentProcessIdentifier() const noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] details  The details to copy to this.
    /// @result A deep copy of the input details.
    ModuleDetails& operator=(const ModuleDetails &details);
    /// @brief Move assignment operator.
    /// @param[in,out] details  The details whose memory will be moved to this.
    ///                         On exit, details's behavior is undefined.
    /// @result The memory from details moved to this.
    ModuleDetails& operator=(ModuleDetails &&details) noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~ModuleDetails();
    /// @}
private:
    class ModuleDetailsImpl;
    std::unique_ptr<ModuleDetailsImpl> pImpl;
};
std::ostream& operator<<(std::ostream &os, const ModuleDetails &details);
}
#endif