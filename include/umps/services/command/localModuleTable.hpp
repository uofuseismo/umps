#ifndef UMPS_SERVICES_COMMAND_LOCAL_MODULE_TABLE_HPP
#define UMPS_SERVICES_COMMAND_LOCAL_MODULE_TABLE_HPP
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

    /// @name Open Table
    /// @{

    /// @brief Opens the process table in read-write mode.
    /// @param[in] tableFile            The table file name which will store
    ///                                 the local process information.
    /// @param[in] createIfDoesNotExit  If the table does not exist then
    ///                                 create it.
    /// @throws std::invalid_argument if createIfDoesNotExist is false and the
    ///         table does not exist.
    /// @throws std::runtime_error if errors are encountered in creating
    ///         the table.
    void open(const std::string &tableFile,
              bool createIfDoesNotExist = true);
    /// @brief Opens the default table.
    /// @param[in] createIfDoesNotExit  If the table does not exist then
    ///                                 create it. 
    void open(bool createIfDoesNotExist = true);
    /// @brief Opens the table in read-only mode.
    /// @param[in] tableFile  The name of the file with the table.
    /// @throws std::invalid_argument if the file does not exist.
    void openReadOnly(const std::string &tableFile);
    /// @brief Opens the default table in read-only mode.
    void openReadOnly();
    /// @result True indicates the table is open.
    [[nodiscard]] bool isOpen() const noexcept;
    /// @result True indicates the table is read-only.
    [[nodiscard]] bool isReadOnly() const;
    /// @}

    /// @name Check Module Exists
    /// @{
 
    /// @result True indicates the module exists.
    /// @param[in] details  The module to determine if in the table.
    /// @throws std::runtime_error if \c isOpen() is false.
    /// @throws std::invalid_argument if details.haveName() is false.
    //[[nodiscard]] bool haveModule(const LocalModuleDetails &details) const;
    /// @result True indicates the module exists.
    /// @param[in] moduleName  The name of module to determine if in the table. 
    /// @throws std::runtime_error if \c isOpen() is false.
    [[nodiscard]] bool haveModule(const std::string &moduleName) const;
    /// @}

    /// @name Add/Update/Delete Module
    /// @{

    /// @brief Adds a module.  
    /// @param[in] details  The module details.
    /// @throws std::invalid_argument if the module name is not set.
    /// @throws std::invalid_argument if \c haveModule() is true.
    void addModule(const LocalModuleDetails &details);

    /// @brief Update module.
    /// @param[in] details  The module details to update.
    /// @throws std::invalid_argument if the module name is not set.
    /// @note If the module does not exist then it will be added.
    void updateModule(const LocalModuleDetails &details);

    /// @brief Deletes a module given its name.
    /// @param[in] details  The module details containing the module name.
    /// @throws std::invalid_argument if details.haveName() is false.
    void deleteModule(const LocalModuleDetails &details);
    /// @param[in] moduleName  The name of the module to delete.
    void deleteModule(const std::string &moduleName);
    /// @}

    /// @brief Queries all modules.
    /// @result All the local modules in the module table.
    [[nodiscard]] std::vector<LocalModuleDetails> queryAllModules() const;
    /// @result Queries a module.
    /// @param[in] name  The name of the module to query.
    /// @throws std::runtime_error if \c haveModule() is false.
    [[nodiscard]] LocalModuleDetails queryModule(const std::string &name) const;

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
