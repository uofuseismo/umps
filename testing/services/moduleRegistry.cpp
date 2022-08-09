#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "umps/services/moduleRegistry/registeredModulesRequest.hpp"
#include "umps/services/moduleRegistry/registeredModulesResponse.hpp"
#include "umps/services/moduleRegistry/registrationRequest.hpp"
#include "umps/services/moduleRegistry/registrationResponse.hpp"
#include "umps/services/moduleRegistry/moduleDetails.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>
namespace
{
namespace URegistry = UMPS::Services::ModuleRegistry;
namespace UAuth = UMPS::Authentication;

bool operator==(const URegistry::ModuleDetails &lhs,
                const URegistry::ModuleDetails &rhs)
{
    if (lhs.getExecutableName() != rhs.getExecutableName()){return false;}
    if (lhs.getName() != rhs.getName()){return false;}
    if (lhs.getMachine() != rhs.getMachine()){return false;}
    if (lhs.getProcessIdentifier() != rhs.getProcessIdentifier()){return false;}
    if (lhs.getParentProcessIdentifier() !=
        rhs.getParentProcessIdentifier()){return false;}
    return true;
}

TEST(ModuleRegistry, ModuleDetails)
{
    const std::string moduleName{"TestModule"};
    const std::string machine{"TestMachine"};
    const std::string executable{"TestApp"};
    int64_t pid{1234};
    int64_t ppid{12345};
    URegistry::ModuleDetails details;
 
    EXPECT_NO_THROW(details.setName(moduleName));
    details.setExecutableName(executable);
    details.setMachine(machine);
    details.setProcessIdentifier(pid);
    details.setParentProcessIdentifier(ppid);

    URegistry::ModuleDetails dcopy(details);
    EXPECT_EQ(details.getExecutableName(), executable);
    EXPECT_EQ(details.getName(), moduleName);
    EXPECT_EQ(details.getMachine(), machine);
    EXPECT_EQ(details.getProcessIdentifier(), pid);
    EXPECT_EQ(details.getParentProcessIdentifier(), ppid);
}

TEST(ModuleRegistry, RegistrationRequest)
{
    URegistry::RegistrationRequest request;
    const std::string messageType{"UMPS::Services::ModuleRegistry::RegistrationRequest"};
    const std::string executable{"TestApp"};
    const std::string moduleName{"TestModule"};
    const std::string machine{"TestMachine"};
    const int64_t pid{1295};
    const int64_t ppid{99492};
    const uint64_t identifier{4586};

    URegistry::ModuleDetails details;
    details.setExecutableName(executable);
    EXPECT_NO_THROW(details.setName(moduleName));
    details.setProcessIdentifier(pid);
    details.setParentProcessIdentifier(ppid);
    details.setMachine(machine);
 
    EXPECT_NO_THROW(request.setModuleDetails(details));
    request.setIdentifier(identifier);    

    auto message = request.toMessage();
    URegistry::RegistrationRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(message));
    EXPECT_TRUE(details == rCopy.getModuleDetails());
    //EXPECT_EQ(rCopy.getModuleDetails().getExecutableName(), executable);
    //EXPECT_EQ(rCopy.getModuleDetails().getName(), moduleName);
    //EXPECT_EQ(rCopy.getModuleDetails().getMachine(), machine);
    //EXPECT_EQ(rCopy.getModuleDetails().getProcessIdentifier(), pid);
    //EXPECT_EQ(rCopy.getModuleDetails().getParentProcessIdentifier(), ppid);
    EXPECT_EQ(rCopy.getIdentifier(), identifier);
    EXPECT_EQ(rCopy.getMessageType(), messageType);

    request.clear();
    EXPECT_EQ(request.getMessageType(), messageType);
}

TEST(ModuleRegistry, RegistrationResponse)
{
    URegistry::RegistrationResponse response;
    const std::string messageType{"UMPS::Services::ModuleRegistry::RegistrationResponse"};
    const URegistry::RegistrationReturnCode
        code{URegistry::RegistrationReturnCode::Exists};
    const uint64_t identifier{4586};

    response.setReturnCode(code);
    response.setIdentifier(identifier);

    URegistry::RegistrationResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(rCopy.getReturnCode(), code);
    EXPECT_EQ(rCopy.getIdentifier(), identifier);
    EXPECT_EQ(rCopy.getMessageType(), messageType);

    response.clear();
    EXPECT_EQ(response.getMessageType(), messageType);
} 

TEST(ModuleRegistry, RegisteredModulesRequest)
{
    const std::string messageType{"UMPS::Services::ModuleRegistry::RegisteredModulesRequest"};
    URegistry::RegisteredModulesRequest request;
    const uint64_t identifier{99422};
    request.setIdentifier(identifier);

    URegistry::RegisteredModulesRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getIdentifier(), identifier);
    EXPECT_EQ(rCopy.getMessageType(), messageType);

    request.clear();
    EXPECT_EQ(request.getMessageType(), messageType);
}

TEST(ModuleRegistry, RegisteredModulesResponse)
{
    const std::string messageType{"UMPS::Services::ModuleRegistry::RegisteredModulesResponse"};
    const std::vector<std::string> executables{"TestApp1", "TestApp2", "TestApp1"};
    const std::vector<std::string> moduleNames{"TestModule1", "TestModule2", "TestModule3"};
    const std::vector<std::string> machines{"TestMachine1", "TestMachine2", "TestMachine2"};
    const std::vector<int64_t> pids{1295, 1296, 1297};
    const std::vector<int64_t> ppids{99492, 99329, 99593};
    const uint64_t identifier{28239};
    const auto returnCode = URegistry::RegisteredModulesReturnCode::Success;

    std::vector<URegistry::ModuleDetails> modules;
    for (int i = 0; i < static_cast<int> (machines.size()); ++i)
    {
        URegistry::ModuleDetails details; 
        details.setExecutableName(executables.at(i));
        details.setName(moduleNames.at(i));
        details.setProcessIdentifier(pids.at(i));
        details.setParentProcessIdentifier(ppids.at(i));
        details.setMachine(machines.at(i));
        modules.push_back(details);
    }

    URegistry::RegisteredModulesResponse response;
    response.setModules(modules);
    response.setReturnCode(returnCode);
    response.setIdentifier(identifier);     

    URegistry::RegisteredModulesResponse rCopy;
    rCopy.fromMessage(response.toMessage());
    EXPECT_EQ(rCopy.getReturnCode(), returnCode);
    EXPECT_EQ(rCopy.getIdentifier(), identifier); 
    auto modulesBack = rCopy.getModules();
    EXPECT_EQ(modulesBack.size(), modules.size());
    for (int i = 0; i < static_cast<int> (modules.size()); ++i)
    {
        bool foundMatch = false;
        for (int j = 0; j < static_cast<int> (modules.size()); ++j)
        {
            if (modulesBack[i].getProcessIdentifier() ==
                modules[j].getProcessIdentifier())
            {
                EXPECT_TRUE(modulesBack[i] == modules[j]);
                foundMatch = true;
            }
        } 
        EXPECT_TRUE(foundMatch);
    }

    response.clear();
    EXPECT_EQ(response.getMessageType(), messageType);
}

}
