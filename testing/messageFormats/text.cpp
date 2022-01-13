#include <iostream>
#include <string>
#include "umps/messageFormats/text.hpp"
#include <gtest/gtest.h>
namespace
{

#define MESSAGE_TYPE "UMPS::MessageFormats::Text"

using namespace UMPS::MessageFormats;

TEST(TextTest, Text)
{
    Text text;
    const std::string contents = "This is the contents of a message.\n  This test recovers this.";

    text.setContents(contents);
    EXPECT_EQ(text.getMessageType(), MESSAGE_TYPE);

    Text textCopy(text);
    EXPECT_EQ(textCopy.getContents(), contents);

    auto textMessage = textCopy.toMessage();
    textCopy.clear();
    EXPECT_TRUE(textCopy.getContents().empty());
    textCopy.fromMessage(textMessage.data(), textMessage.size());  
    EXPECT_EQ(textCopy.getContents(), contents); 
}

}
