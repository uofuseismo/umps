#ifndef PRIVATE_SERVICES_PACKDETAILS_HPP
#define PRIVATE_SERVICES_PACKDETAILS_HPP
#include <nlohmann/json.hpp>
#include "umps/services/moduleRegistry/moduleDetails.hpp"
namespace
{
[[nodiscard]] nlohmann::json
    pack(const UMPS::Services::ModuleRegistry::ModuleDetails &details)
{
    nlohmann::json obj;
    obj["ModuleName"] = details.getName(); // Throws
    obj["Executable"] = details.getExecutableName();
    obj["ProcessIdentifier"] = details.getProcessIdentifier();
    obj["ParentProcessIdentifier"] = details.getParentProcessIdentifier();
    obj["Machine"] = details.getMachine();
    return obj;
}
[[nodiscard]] UMPS::Services::ModuleRegistry::ModuleDetails
    unpack(const nlohmann::json &obj)
{
    UMPS::Services::ModuleRegistry::ModuleDetails details;
    details.setName(obj["ModuleName"].get<std::string> ());
    details.setExecutableName(obj["Executable"].get<std::string> ());
    details.setProcessIdentifier(obj["ProcessIdentifier"].get<int64_t> ());
    details.setParentProcessIdentifier(obj["ParentProcessIdentifier"].get<int64_t> ());
    details.setMachine(obj["Machine"].get<std::string> ());
    return details;
}
}
#endif
