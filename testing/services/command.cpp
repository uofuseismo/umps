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
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::Command;

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

    EXPECT_FALSE(table.haveModule(details1));
    EXPECT_NO_THROW(table.addModule(details1));
    EXPECT_TRUE(table.haveModule(details1)); 

    EXPECT_FALSE(table.haveModule(details2));
    EXPECT_NO_THROW(table.addModule(details2));
    EXPECT_TRUE(table.haveModule(details2));

    auto allModules = table.queryAllModules();
    EXPECT_EQ(allModules.size(), allModulesRef.size()); 
    for (const auto &mRef : allModulesRef)
    {
        bool lmatch = false;
        for (const auto &m : allModules)
        {
            if (m.getName() == mRef.getName() &&
                m.getIPCDirectory() == mRef.getIPCDirectory() &&
                m.getProcessIdentifier() == mRef.getProcessIdentifier() &&
                m.getApplicationStatus() == mRef.getApplicationStatus()) 
            {
                lmatch = true;
            }
        }
        EXPECT_TRUE(lmatch);
    }

    EXPECT_NO_THROW(table.deleteModule(details2));
  
    if (std::filesystem::exists(tableName))
    {
        std::filesystem::remove(tableName);
    }
}

}
