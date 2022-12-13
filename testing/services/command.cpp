#include <string>
#include <filesystem>
#include <chrono>
#include "umps/services/command/moduleDetails.hpp"
#include "umps/services/command/requestorOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/moduleTable.hpp"
#include "umps/services/command/terminateRequest.hpp"
#include "umps/services/command/terminateResponse.hpp"
#include "umps/proxyServices/command/availableModulesRequest.hpp"
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/proxyServices/command/registrationRequest.hpp"
#include "umps/proxyServices/command/registrationResponse.hpp"
#include "umps/proxyServices/command/requestorOptions.hpp"
#include "umps/proxyServices/command/replierOptions.hpp"
#include "umps/proxyServices/command/proxyOptions.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::Command;
using namespace UMPS::ProxyServices::Command;

bool operator==(const UMPS::Services::Command::ModuleDetails &a,
                const UMPS::Services::Command::ModuleDetails &b)
{
    if (a.haveName() == b.haveName())
    {
        if (a.haveName())
        {
            if (a.getName() != b.getName()){return false;}
        }
    }
    else
    {
        return false;
    }
    if (a.getIPCDirectory() != b.getIPCDirectory()){return false;}     
    if (a.getProcessIdentifier() != b.getProcessIdentifier()){return false;}
    if (a.getApplicationStatus() != b.getApplicationStatus()){return false;}
    return true;
}

bool operator==(const UMPS::ProxyServices::Command::ModuleDetails &a,
                const UMPS::ProxyServices::Command::ModuleDetails &b)
{
    if (a.haveName() == b.haveName())
    {
        if (a.haveName())
        {
            if (a.getName() != b.getName()){return false;}
        }
    }
    else
    {
        return false;
    }
    if (a.getInstance() != b.getInstance()){return false;}
    if (a.getExecutableName() != b.getExecutableName()){return false;}
    if (a.getMachine() != b.getMachine()){return false;}
    if (a.getProcessIdentifier() != b.getProcessIdentifier()){return false;}
    if (a.getParentProcessIdentifier() != b.getParentProcessIdentifier())
    {
        return false;
    }
    return true;
}

TEST(Command, ProxyOptions)
{
    UMPS::Authentication::ZAPOptions zapOptions;
    zapOptions.setStrawhouseServer();
    std::vector<std::chrono::milliseconds> pingIntervals
    {
        std::chrono::milliseconds {20},
        std::chrono::milliseconds {15},
        std::chrono::milliseconds {35}
    };
    std::vector<std::chrono::milliseconds> pingIntervalsSorted
    {
        std::chrono::milliseconds {15},
        std::chrono::milliseconds {20},
        std::chrono::milliseconds {35}
    };  

    const std::string frontend{"tcp://127.0.0.1:8080"};
    const std::string backend{"tcp://127.0.0.2:8080"};
    const int frontendHWM{100};
    const int backendHWM{200};
    ProxyOptions options;
    EXPECT_NO_THROW(options.setFrontendAddress(frontend));
    EXPECT_NO_THROW(options.setBackendAddress(backend));
    EXPECT_NO_THROW(options.setPingIntervals(pingIntervals));
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendHighWaterMark(backendHWM);
    options.setZAPOptions(zapOptions);

    ProxyOptions cOptions(options);
    EXPECT_EQ(cOptions.getFrontendAddress(), frontend);
    EXPECT_EQ(cOptions.getBackendAddress(), backend);
    EXPECT_EQ(cOptions.getFrontendHighWaterMark(), frontendHWM);
    EXPECT_EQ(cOptions.getBackendHighWaterMark(), backendHWM);
    EXPECT_EQ(cOptions.getPingIntervals(), pingIntervalsSorted);
}

TEST(Command, ModuleDetails)
{
    UMPS::Services::Command::ModuleDetails details;
    const std::string moduleName{"testModule"};
    const std::string directory{"./a/b/c"};
    const std::string ipcFile{"./a/b/c/testModule.ipc"};
    const int64_t processIdentifier{430};
    const ApplicationStatus applicationStatus{ApplicationStatus::Running};

    details.setName(moduleName);
    details.setIPCDirectory(directory);
    details.setProcessIdentifier(processIdentifier);
    details.setApplicationStatus(applicationStatus);

    UMPS::Services::Command::ModuleDetails dCopy(details);
    EXPECT_EQ(dCopy.getName(), moduleName);
    EXPECT_EQ(dCopy.getIPCDirectory(), directory);
    EXPECT_EQ(dCopy.getProcessIdentifier(), processIdentifier);
    EXPECT_EQ(dCopy.getApplicationStatus(), applicationStatus);
    EXPECT_EQ(dCopy.getIPCFileName(), ipcFile); 
}

TEST(ProxyCommand, RequestorOptions)
{
    const std::string address = "tcp://127.0.0.1:8080";
    const std::chrono::milliseconds timeOut{234}; 
    UMPS::ProxyServices::Command::RequestorOptions options;
    EXPECT_NO_THROW(options.setAddress(address));
    options.setReceiveTimeOut(timeOut);

    UMPS::ProxyServices::Command::RequestorOptions copy(options);
    auto rOptions = copy.getOptions();
    EXPECT_EQ(rOptions.getAddress(), address);
    EXPECT_EQ(rOptions.getTimeOut(), timeOut);
}

TEST(Command, RequestorOptions)
{
    const std::string moduleName{"example"};
    const std::string directory{"./"};
    const std::string ipcFileName{"./example.ipc"};
    const std::string address = "ipc://./example.ipc";
    const std::chrono::milliseconds timeOut{234};
    UMPS::Services::Command::RequestorOptions options;
    EXPECT_NO_THROW(options.setModuleName(moduleName));
    EXPECT_NO_THROW(options.setIPCDirectory(directory));
    options.setReceiveTimeOut(timeOut);

    UMPS::Services::Command::RequestorOptions copy(options);
    EXPECT_EQ(copy.getModuleName(), moduleName);
    EXPECT_EQ(copy.getIPCDirectory(), directory);
    EXPECT_EQ(copy.getIPCFileName(), ipcFileName);

    options.clear();
    auto rOptions = copy.getOptions().getOptions();
    EXPECT_EQ(rOptions.getAddress(), address);
    EXPECT_EQ(rOptions.getTimeOut(), timeOut);
}


TEST(Command, AvailableCommandsRequest)
{
    AvailableCommandsRequest request;

    AvailableCommandsRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::Services::Command::AvailableCommandsRequest");
}

TEST(Command, AvailableCommandsResponse)
{
    AvailableCommandsResponse response;
    const std::string commands = R"(
Test program.

These are the options:
    help   Prints this message.
    hello  Says hi.
    quit   Terminates the program.
)";
    response.setCommands(commands);

    AvailableCommandsResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(response.getCommands(), commands);

    response.clear();
    EXPECT_EQ(response.getMessageType(),
              "UMPS::Services::Command::AvailableCommandsResponse");

}

TEST(Command, CommandRequest)
{
    CommandRequest request;
    const std::string command{"say hello"};
    EXPECT_NO_THROW(request.setCommand(command));

    CommandRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getCommand(), command);
 
    request.clear();
    EXPECT_EQ(request.getMessageType(),
              "UMPS::Services::Command::CommandRequest");
}

TEST(Command, CommandResponse)
{
    CommandResponse response;
    const std::string responseString{"howdy"};
    const CommandReturnCode returnCode = CommandReturnCode::InvalidCommand;
    EXPECT_NO_THROW(response.setResponse(responseString));
    response.setReturnCode(returnCode);

    CommandResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(rCopy.getResponse(), responseString);
    EXPECT_EQ(rCopy.getReturnCode(), returnCode);
 
    response.clear();
    EXPECT_EQ(response.getMessageType(),
              "UMPS::Services::Command::CommandResponse");
}

TEST(Command, TerminateRequest)
{
    TerminateRequest request;

    TerminateRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::Services::Command::TerminateRequest");
}

TEST(Command, TerminateResponse)
{
    TerminateResponse response;
    const auto returnCode = TerminateResponse::ReturnCode::ApplicationError;
    response.setReturnCode(returnCode);

    TerminateResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(rCopy.getReturnCode(), returnCode);
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::Services::Command::TerminateResponse");
}

TEST(Command, RegistrationRequest)
{
    UMPS::ProxyServices::Command::ModuleDetails details;
    RegistrationType registrationType{RegistrationType::Deregister};
    details.setName("TestModule");
    details.setExecutableName("testBinary");
    details.setInstance(4);
    details.setMachine("host.name");
    details.setProcessIdentifier(4832);
    details.setParentProcessIdentifier(83823);

    RegistrationRequest request;
    EXPECT_NO_THROW(request.setModuleDetails(details));
    request.setRegistrationType(RegistrationType::Deregister);

    RegistrationRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_TRUE(rCopy.getModuleDetails() == details);
    EXPECT_EQ(rCopy.getRegistrationType(), registrationType);
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::ProxyServices::Command::RegistrationRequest");
}

TEST(Command, RegistrationResponse)
{
    RegistrationResponse response;
    const RegistrationReturnCode
        returnCode{RegistrationReturnCode::InvalidRequest};
    response.setReturnCode(returnCode);

    RegistrationResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(rCopy.getReturnCode(), returnCode);
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::ProxyServices::Command::RegistrationResponse");
}

TEST(ProxyCommand, ModuleDetails)
{
    const std::string moduleName{"TestModule"};
    const std::string machine{"TestMachine"};
    const std::string executable{"TestApp"};
    const uint16_t instance{30};
    int64_t pid{1234};
    int64_t ppid{12345};
    UMPS::ProxyServices::Command::ModuleDetails details;
 
    EXPECT_NO_THROW(details.setName(moduleName));
    details.setInstance(instance);
    details.setExecutableName(executable);
    details.setMachine(machine);
    details.setProcessIdentifier(pid);
    details.setParentProcessIdentifier(ppid);

    UMPS::ProxyServices::Command::ModuleDetails dcopy(details);
    EXPECT_EQ(details.getInstance(), instance);
    EXPECT_EQ(details.getExecutableName(), executable);
    EXPECT_EQ(details.getName(), moduleName);
    EXPECT_EQ(details.getMachine(), machine);
    EXPECT_EQ(details.getProcessIdentifier(), pid);
    EXPECT_EQ(details.getParentProcessIdentifier(), ppid);
}

TEST(Command, AvailableModulesRequest)
{
    const int64_t identifier{48233};
    AvailableModulesRequest request;
    request.setIdentifier(identifier);

    AvailableModulesRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getIdentifier(), identifier);
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::ProxyServices::Command::AvailableModulesRequest");

    request.clear();
    EXPECT_EQ(request.getIdentifier(), 0); 
}

TEST(Command, AvailableModulesResponse)
{
    std::vector<UMPS::ProxyServices::Command::ModuleDetails> details(2);
    details[0].setName("mod1");
    details[0].setExecutableName("exec1");
    details[0].setMachine("test1.machine.com");
    details[0].setProcessIdentifier(3234);
    details[0].setParentProcessIdentifier(3245);

    details[1].setName("mod2");
    details[1].setExecutableName("exec2");
    details[0].setMachine("test2.machine.com");
    details[1].setProcessIdentifier(4234);
    details[1].setParentProcessIdentifier(4245);


    const int64_t identifier{83832};
    AvailableModulesResponse response;
    response.setIdentifier(identifier);
    EXPECT_NO_THROW(response.setModules(details));

    AvailableModulesResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(rCopy.getIdentifier(), identifier); 
    auto dCopy = rCopy.getModules();
    EXPECT_EQ(details.size(), dCopy.size());
    for (const auto &d1 : details)
    {
        bool lMatch{false};
        for (const auto &d2 : dCopy)
        {
            if (d1 == d2){lMatch = true;}
        }
        EXPECT_TRUE(lMatch);
    }
    response.clear();
    EXPECT_EQ(response.getMessageType(),
              "UMPS::ProxyServices::Command::AvailableModulesResponse");
}


std::unique_ptr<UMPS::MessageFormats::IMessage>
    dumbyCallback(const std::string &, const void *, const size_t)
{
    return nullptr;
}

TEST(Command, ReplierOptions)
{
    const std::chrono::milliseconds timeOut{12};
    int hwm = 16;
    const std::string address{"tcp://127.0.0.1:8090"};
    UMPS::Authentication::ZAPOptions zapOptions;
    zapOptions.setStrawhouseServer();
    UMPS::ProxyServices::Command::ModuleDetails details;
    details.setName("Test");
    details.setInstance(1);
    details.setMachine("test.box");
    details.setParentProcessIdentifier(2);
    details.setProcessIdentifier(3);

    UMPS::ProxyServices::Command::ReplierOptions options;
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setModuleDetails(details));
    options.setZAPOptions(zapOptions);
    EXPECT_NO_THROW(options.setPollingTimeOut(timeOut));
    EXPECT_NO_THROW(options.setHighWaterMark(hwm));
    options.setCallback(std::bind(&dumbyCallback,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3));

    UMPS::ProxyServices::Command::ReplierOptions copy(options);
    EXPECT_EQ(copy.getOptions().getAddress(), address);
    EXPECT_EQ(copy.getOptions().getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    EXPECT_EQ(copy.getOptions().getPollingTimeOut(), timeOut);
    EXPECT_EQ(copy.getOptions().getSendHighWaterMark(), hwm);
    EXPECT_TRUE(copy.getModuleDetails() == details);

    options.clear();
    zapOptions.clear();
    // TODO should make getters/setters 
    //EXPECT_EQ(options.getOptions().getPollingTimeOut(),
    //          std::chrono::milliseconds {10});
    //EXPECT_EQ(options.getOptions().getZAPOptions().getSecurityLevel(),
    //          zapOptions.getSecurityLevel()); 
    EXPECT_FALSE(options.haveAddress());
    EXPECT_FALSE(options.haveModuleDetails());
    //EXPECT_EQ(options.getOptions().getSendHighWaterMark(), 0);
} 

TEST(Command, ModuleTable)
{
    ModuleTable table;
    std::string tableName{"localModuleTable.sqlite3"};
    bool createIfDoesNotExist = true;
    table.open(tableName, createIfDoesNotExist);

    const std::string moduleName1{"testModule"};
    const std::string ipcDirectory1{"./"};
    const int64_t processIdentifier1{829};
    const ApplicationStatus applicationStatus1{ApplicationStatus::Running};

    const std::string moduleName2{"anotherTestModule"};
    const std::string ipcDirectory2{"./"};
    const int64_t processIdentifier2{19392};
    const ApplicationStatus applicationStatus2{ApplicationStatus::Running};

    const std::string moduleName3{"anotherTestModule"};
    const std::string ipcDirectory3{"./"};
    const int64_t processIdentifier3{processIdentifier1 + processIdentifier2};
    const ApplicationStatus applicationStatus3{ApplicationStatus::Unknown};

    std::vector<UMPS::Services::Command::ModuleDetails> allModulesRef;
    UMPS::Services::Command::ModuleDetails details1;
    details1.setName(moduleName1);
    details1.setIPCDirectory(ipcDirectory1);
    details1.setProcessIdentifier(processIdentifier1);
    details1.setApplicationStatus(applicationStatus1);
    allModulesRef.push_back(details1);

    UMPS::Services::Command::ModuleDetails details2;
    details2.setName(moduleName2);
    details2.setIPCDirectory(ipcDirectory2);
    details2.setProcessIdentifier(processIdentifier2);
    details2.setApplicationStatus(applicationStatus2);
    allModulesRef.push_back(details2);

    UMPS::Services::Command::ModuleDetails details3;
    details3.setName(moduleName3);
    details3.setIPCDirectory(ipcDirectory3);
    details3.setProcessIdentifier(processIdentifier3);
    details3.setApplicationStatus(applicationStatus3);

    EXPECT_FALSE(table.haveModule(details1.getName()));
    EXPECT_NO_THROW(table.addModule(details1));
    EXPECT_TRUE(table.haveModule(details1.getName())); 

    EXPECT_FALSE(table.haveModule(details2.getName()));
    EXPECT_NO_THROW(table.addModule(details2));
    EXPECT_TRUE(table.haveModule(details2.getName()));

    auto allModules = table.queryAllModules();
    EXPECT_EQ(allModules.size(), allModulesRef.size()); 
    for (const auto &mRef : allModulesRef)
    {
        bool lmatch = false;
        for (const auto &m : allModules)
        {
            if (mRef == m){lmatch = true;}
            /*
            if (m.getName() == mRef.getName() &&
                m.getIPCDirectory() == mRef.getIPCDirectory() &&
                m.getProcessIdentifier() == mRef.getProcessIdentifier() &&
                m.getApplicationStatus() == mRef.getApplicationStatus()) 
            {
                lmatch = true;
            }
            */
        }
        EXPECT_TRUE(lmatch);
    }

    EXPECT_NO_THROW(table.deleteModule(details2));
    EXPECT_FALSE(table.haveModule(details2.getName()));
    EXPECT_NO_THROW(table.addModule(details2));
    EXPECT_TRUE(table.haveModule(details2.getName()));
    EXPECT_NO_THROW(table.updateModule(details3));
    EXPECT_TRUE(table.haveModule(details3.getName()));
 
    allModules = table.queryAllModules();
    allModulesRef.at(0) = details1;
    allModulesRef.at(1) = details3; 
    EXPECT_EQ(allModules.size(), allModulesRef.size());
    for (const auto &mRef : allModulesRef)
    {   
        bool lmatch = false;
        for (const auto &m : allModules)
        {
            if (mRef == m){lmatch = true;}
            /*
            if (m.getName() == mRef.getName() &&
                m.getIPCDirectory() == mRef.getIPCDirectory() &&
                m.getProcessIdentifier() == mRef.getProcessIdentifier() &&
                m.getApplicationStatus() == mRef.getApplicationStatus()) 
            {
                lmatch = true;
            }
            */
        }
        EXPECT_TRUE(lmatch);
    }

    auto details3Back = table.queryModule(details3.getName());
    EXPECT_TRUE(details3Back == details3);
 
    if (std::filesystem::exists(tableName))
    {
        std::filesystem::remove(tableName);
    }
}

}
