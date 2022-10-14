#ifndef PRIVATE_SERVICES_MODULEDETAILS_HPP
#define PRIVATE_SERVICES_MODULEDETAILS_HPP
#include <nlohmann/json.hpp>
#include "umps/proxyServices/command/moduleDetails.hpp"
namespace
{
[[nodiscard]] nlohmann::json
    pack(const UMPS::ProxyServices::Command::ModuleDetails &details)
{
    nlohmann::json obj;
    obj["ModuleName"] = details.getName(); // Throws
    obj["Executable"] = details.getExecutableName();
    obj["Instance"] = static_cast<uint16_t> (details.getInstance());
    obj["ProcessIdentifier"] = details.getProcessIdentifier();
    obj["ParentProcessIdentifier"] = details.getParentProcessIdentifier();
    obj["Machine"] = details.getMachine();
    return obj;
}
[[nodiscard]] UMPS::ProxyServices::Command::ModuleDetails
    unpack(const nlohmann::json &obj)
{
    UMPS::ProxyServices::Command::ModuleDetails details;
    details.setName(obj["ModuleName"].get<std::string> ());
    details.setExecutableName(obj["Executable"].get<std::string> ());
    details.setInstance(obj["Instance"].get<uint16_t> ());
    details.setProcessIdentifier(obj["ProcessIdentifier"].get<int64_t> ());
    details.setParentProcessIdentifier(obj["ParentProcessIdentifier"].get<int64_t> ());
    details.setMachine(obj["Machine"].get<std::string> ());
    return details;
}
}
#endif
