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


    /// @brief Opens the process table.
    /// @param[in] tableFile            The table file name which will store
    ///                                 the local process information.
    /// @param[in] createIfDoesNotExit  If the table does not exist then
    ///                                 create it. 
    void open(const std::string &tableFile,
              bool createIfDoesNotExist = true);

    /// @brief 
    void addModule(const LocalModuleDetails &details);

    /// @names Destructors
    /// @{
    
    /// @brief Destructor.
    ~LocalModuleTable();
    /// @}
private:
    class LocalModuleTableImpl;
    std::unique_ptr<LocalModuleTableImpl> pImpl;
};
}
#endif
