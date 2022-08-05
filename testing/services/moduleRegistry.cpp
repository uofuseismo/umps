#include <iostream>
#include <fstream>
#include <string>
#include "umps/services/moduleRegistry/registrationRequest.hpp"
#include "umps/services/moduleRegistry/registrationResponse.hpp"
#include "umps/services/moduleRegistry/moduleDetails.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>
namespace
{
namespace URegistry = UMPS::Services::ModuleRegistry;
namespace UAuth = UMPS::Authentication;

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
    EXPECT_EQ(rCopy.getModuleDetails().getExecutableName(), executable);
    EXPECT_EQ(rCopy.getModuleDetails().getName(), moduleName);
    EXPECT_EQ(rCopy.getModuleDetails().getMachine(), machine);
    EXPECT_EQ(rCopy.getModuleDetails().getProcessIdentifier(), pid);
    EXPECT_EQ(rCopy.getModuleDetails().getParentProcessIdentifier(), ppid);
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

}
