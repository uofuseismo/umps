#ifndef UMPS_SERVICES_COMMAND_MODULE_DETAILS_HPP
#define UMPS_SERVICES_COMMAND_MODULE_DETAILS_HPP
#include <memory>
namespace UMPS::Services::Command
{
/// @class ModuleDetails "moduleDetails.hpp" "umps/services/command/moduleDetails.hpp"
/// @brief Defines the locally running module properties.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_uLocalCommand
class ModuleDetails
{
public:
    /// @brief Defines the application's status as running/not-running/undefined.
    enum class ApplicationStatus : int 
    {
        Running = 0,     /*!< This indicates the application is running. */
        NotRunning = 1,  /*!< This indicates the application is not running. */
        Unknown = 2      /*!< This indicates the application's status is unknown. */
    };
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

    /// @brief Sets the identifier of the process running this module.
    /// @param[in] identifier  The process identifier.
    void setProcessIdentifier(int64_t identifier) noexcept;
    /// @result The identifier of the process running this module.
    [[nodiscard]] int64_t getProcessIdentifier() const noexcept;

    /// @brief Sets the application's status.
    /// @param[in] status  The applications status.
    void setApplicationStatus(ApplicationStatus status) noexcept;
    /// @result The application's status.
    [[nodiscard]] ApplicationStatus getApplicationStatus() const noexcept;

    /// @brief Sets the directory where the IPC files will exist.
    /// @param[in] directory   The IPC directory.
    void setIPCDirectory(const std::string &directory);
    /// @result The directory where the IPC files will exist.
    [[nodiscard]] std::string getIPCDirectory() const noexcept;
    /// @}

    /// @result The IPC file name.
    /// @throws std::runtime_error if \c haveName() is false.
    [[nodiscard]] std::string getIPCFileName() const;

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
}
#endif
