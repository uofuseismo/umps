#include <string>
#include <chrono>
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/services/command/commandsRequest.hpp"
#include "umps/services/command/commandsResponse.hpp"
#include "umps/services/command/textRequest.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::Command;

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
    CommandsRequest request;

    CommandsRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getMessageType(),
              "UMPS::Services::Command::CommandsRequest");
}

TEST(Command, CommandsResponse)
{
    CommandsResponse response;
    const std::string commands = R"(
Test program.

These are the options:
    help   Prints this message.
    hello  Says hi.
    quit   Terminates the program.
)";
    response.setCommands(commands);

    CommandsResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(response.toMessage()));
    EXPECT_EQ(response.getCommands(), commands);

    response.clear();
    EXPECT_EQ(response.getMessageType(),
              "UMPS::Services::Command::CommandsResponse");

}

TEST(Command, TextRequest)
{
    TextRequest request;
    const std::string command = "say hello";
    EXPECT_NO_THROW(request.setCommand(command));

    TextRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(request.toMessage()));
    EXPECT_EQ(rCopy.getCommand(), command);
 
    request.clear();
    EXPECT_EQ(request.getMessageType(),
              "UMPS::Services::Command::TextRequest");
}

}
