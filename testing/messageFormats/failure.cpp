#include <iostream>
#include <string>
#include "umps/messageFormats/failure.hpp"
#include <gtest/gtest.h>
namespace
{

#define MESSAGE_TYPE "UMPS::MessageFormats::Failure"

using namespace UMPS::MessageFormats;

TEST(FailureTest, Failure)
{
    Failure failure;
    const std::string details = "Application failed for no reason.";

    failure.setDetails(details);
    EXPECT_EQ(failure.getMessageType(), MESSAGE_TYPE);

    Failure failureCopy(failure);
    EXPECT_EQ(failureCopy.getDetails(), details);

    auto failureMessage = failureCopy.toMessage();
    failureCopy.clear();
    EXPECT_TRUE(failureCopy.getDetails().empty());
    EXPECT_NO_THROW(failureCopy.fromMessage(failureMessage));
    EXPECT_EQ(failureCopy.getDetails(), details); 
}

}

