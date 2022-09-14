#include <string>
#include <filesystem>
#include <chrono>
#include "umps/services/command/localModuleDetails.hpp"
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/localModuleTable.hpp"
#include "umps/services/command/remoteProxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::Command;

bool operator==(const LocalModuleDetails &a,
                const LocalModuleDetails &b)
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

TEST(Command, RemoteProxyOptions)
{
    UMPS::Authentication::ZAPOptions zapOptions;
    zapOptions.setStrawhouseServer();
    const std::string frontend{"tcp://127.0.0.1:8080"};
    const std::string backend{"tcp://127.0.0.2:8080"};
    const int frontendHWM{100};
    const int backendHWM{200};
    RemoteProxyOptions options;
    EXPECT_NO_THROW(options.setFrontendAddress(frontend));
    EXPECT_NO_THROW(options.setBackendAddress(backend));
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendHighWaterMark(backendHWM);
    options.setZAPOptions(zapOptions);

    RemoteProxyOptions cOptions(options);
    EXPECT_EQ(cOptions.getFrontendAddress(), frontend);
    EXPECT_EQ(cOptions.getBackendAddress(), backend);
    EXPECT_EQ(cOptions.getFrontendHighWaterMark(), frontendHWM);
    EXPECT_EQ(cOptions.getBackendHighWaterMark(), backendHWM);
}

TEST(Command, LocalModuleDetails)
{
    LocalModuleDetails details;
    const std::string moduleName{"testModule"};
    const std::string directory{"./a/b/c"};
    const std::string ipcFile{"./a/b/c/testModule.ipc"};
    const int64_t processIdentifier{430};
    const ApplicationStatus applicationStatus{ApplicationStatus::Running};

    details.setName(moduleName);
    details.setIPCDirectory(directory);
    details.setProcessIdentifier(processIdentifier);
    details.setApplicationStatus(applicationStatus);

    LocalModuleDetails dCopy(details);
    EXPECT_EQ(dCopy.getName(), moduleName);
    EXPECT_EQ(dCopy.getIPCDirectory(), directory);
    EXPECT_EQ(dCopy.getProcessIdentifier(), processIdentifier);
    EXPECT_EQ(dCopy.getApplicationStatus(), applicationStatus);
    EXPECT_EQ(dCopy.getIPCFileName(), ipcFile); 
}

TEST(Command, LocalRequestorCommands)
{
    const std::string moduleName{"example"};
    const std::string directory{"./"};
    const std::string ipcFileName{"./example.ipc"};
    const std::chrono::milliseconds timeOut{234};
    LocalRequestorOptions options;
    EXPECT_NO_THROW(options.setModuleName(moduleName));
    EXPECT_NO_THROW(options.setIPCDirectory(directory));
    options.setReceiveTimeOut(timeOut);

    LocalRequestorOptions copy(options);
    EXPECT_EQ(copy.getModuleName(), moduleName);
    EXPECT_EQ(copy.getReceiveTimeOut(), timeOut);
    EXPECT_EQ(copy.getIPCDirectory(), directory);
    EXPECT_EQ(copy.getIPCFileName(), ipcFileName);

    options.clear();
    EXPECT_EQ(options.getReceiveTimeOut(), std::chrono::milliseconds {10});
}

TEST(Command, CommandsRequest)
{
    AvailableCommandsRequest request;

    AvailableCommandsRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::Services::Command::AvailableCommandsRequest");
}

TEST(Command, CommandsResponse)
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

TEST(Command, LocalModuleTable)
{
    LocalModuleTable table;
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

    std::vector<LocalModuleDetails> allModulesRef;
    LocalModuleDetails details1;
    details1.setName(moduleName1);
    details1.setIPCDirectory(ipcDirectory1);
    details1.setProcessIdentifier(processIdentifier1);
    details1.setApplicationStatus(applicationStatus1);
    allModulesRef.push_back(details1);

    LocalModuleDetails details2;
    details2.setName(moduleName2);
    details2.setIPCDirectory(ipcDirectory2);
    details2.setProcessIdentifier(processIdentifier2);
    details2.setApplicationStatus(applicationStatus2);
    allModulesRef.push_back(details2);

    LocalModuleDetails details3;
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
