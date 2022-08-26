#include <string>
#include <chrono>
#include "umps/services/command/localRequestorOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::Command;

TEST(Command, LocalRequestorOptions)
{
    const std::string moduleName{"example"};
    const std::chrono::milliseconds timeOut{234};
    LocalRequestorOptions options;
    EXPECT_NO_THROW(options.setModuleName(moduleName));
    options.setReceiveTimeOut(timeOut);

    LocalRequestorOptions copy(options);
    EXPECT_EQ(copy.getModuleName(), moduleName);
    EXPECT_EQ(copy.getReceiveTimeOut(), timeOut);

    options.clear();
    EXPECT_EQ(options.getReceiveTimeOut(), std::chrono::milliseconds {10});
}

}
