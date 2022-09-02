#ifndef UMPS_SERVICES_COMMAND_LOCALMODULETABLE_HPP
#define UMPS_SERVICES_COMMAND_LOCALMODULETABLE_HPP
#include <memory>
#include <vector>
namespace UMPS::Services::Command
{
 class LocalModuleDetails;
}
namespace UMPS::Services::Command
{
class LocalModuleTable
{
public:
    /// @names Constructors
    /// @{

    /// @brief Constructor.
    LocalModuleTable();
    /// @}

    /// @brief Opens the process table in read-write mode.
    /// @param[in] tableFile            The table file name which will store
    ///                                 the local process information.
    /// @param[in] createIfDoesNotExit  If the table does not exist then
    ///                                 create it. 
    void open(const std::string &tableFile,
              bool createIfDoesNotExist = true);
    /// @brief Opens the default table.
    /// @param[in] createIfDoesNotExit  If the table does not exist then
    ///                                 create it. 
    void open(bool createIfDoesNotExist = true);

    void openReadOnly(const std::string &tableFile);
    void openReadOnly();
    /// @result True indicates the table is open.
    [[nodiscard]] bool isOpen() const noexcept;

  
    /// @result True indicates the module exists.
    /// @param[in] details  The module to determine if in the table.
    /// @throws std::runtime_error if \c isOpen() is false.
    /// @throws std::invalid_argument if details.haveName() is false.
    [[nodiscard]] bool haveModule(const LocalModuleDetails &details) const;
    /// @result True indicates the module exists.
    /// @param[in] moduleName  The name of module to determine if in the table. 
    /// @throws std::runtime_error if \c isOpen() is false.
    [[nodiscard]] bool haveModule(const std::string &moduleName) const;

    /// @brief Adds a module.  
    /// @param[in] details  The module details.
    /// @throws std::invalid_argument if the module name is not set.
    void addModule(const LocalModuleDetails &details);

    void deleteModule(const LocalModuleDetails &details);
    void deleteModule(const std::string &moduleName);

    /// @brief Queries all modules.
    /// @result All the local modules in the module table.
    [[nodiscard]] std::vector<LocalModuleDetails> queryAllModules() const;

    /// @names Destructors
    /// @{
    
    /// @brief Closes the local module table.
    void close() noexcept;
    /// @brief Destructor.
    ~LocalModuleTable();
    /// @}
private:
    class LocalModuleTableImpl;
    std::unique_ptr<LocalModuleTableImpl> pImpl;
};
}
#endif
