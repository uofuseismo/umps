#include <iostream>
#include <fstream>
#include <string>
#include "umps/services/moduleRegistry/registrationRequest.hpp"
#include "umps/services/moduleRegistry/registrationResponse.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>
namespace
{
namespace URegistry = UMPS::Services::ModuleRegistry;
namespace UAuth = UMPS::Authentication;

TEST(ModuleRegistry, RegistrationRequest)
{
    URegistry::RegistrationRequest request;
    const std::string messageType{"UMPS::Services::ModuleRegistry::RegistrationRequest"};
    const std::string moduleName{"TestModule"};
    const std::string machine{"TestMachine"};
    const uint64_t identifier{4586};

    EXPECT_NO_THROW(request.setModuleName(moduleName));
    request.setMachine(machine);
    request.setIdentifier(identifier);    

    auto message = request.toMessage();
    URegistry::RegistrationRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(message));
    EXPECT_EQ(rCopy.getModuleName(), moduleName);
    EXPECT_EQ(rCopy.getMachine(), machine);
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
