#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace UMPS::MessageFormats;

class Message1 : public IMessage
{
public:
    virtual ~Message1() = default;
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<Message1> (*this);
        return result;
    }
    /// @brief Create a clone of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept
    {
        std::unique_ptr<IMessage> result = std::make_unique<Message1> ();
        return result;
    }
    std::string toMessage() const
    {
        std::string result;
        return result;
    }
    void fromMessage(const std::string &m){fromMessage(m.data(), m.size());}
    void fromMessage(const char *data, const size_t length)
    {
    }
    [[nodiscard]] std::string getMessageType() const noexcept
    {
        return "Message1";
    }
    [[nodiscard]] std::string getMessageVersion() const noexcept
    {
        return "1.0.0";
    }
};

class Message2 : public IMessage
{
public:
    virtual ~Message2() = default;
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const
    {   
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<Message2> (*this);
        return result;
    }   
    /// @brief Create a clone of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept
    {   
        std::unique_ptr<IMessage> result = std::make_unique<Message2> (); 
        return result;
    }   
    std::string toMessage() const
    {   
        std::string result;
        return result;
    }
    void fromMessage(const std::string &m){fromMessage(m.data(), m.size());}
    void fromMessage(const char *data, const size_t length)
    {   
    }   
    [[nodiscard]] std::string getMessageType() const noexcept
    {   
        return "Message2";
    }
    [[nodiscard]] std::string getMessageVersion() const noexcept
    {
        return "2.0.0";
    }
};

class Message3 : public IMessage
{
public:
    virtual ~Message3() = default;
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const
    {   
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<Message3> (*this);
        return result;
    }   
    /// @brief Create a clone of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept
    {   
        std::unique_ptr<IMessage> result = std::make_unique<Message3> (); 
        return result;
    }   
    std::string toMessage() const
    {   
        std::string result;
        return result;
    }
    void fromMessage(const std::string &m){fromMessage(m.data(), m.size());}
    void fromMessage(const char *data, const size_t length)
    {   
    }   
    [[nodiscard]] std::string getMessageType() const noexcept
    {   
        return "Message3";
    }
    [[nodiscard]] std::string getMessageVersion() const noexcept
    {
        return "3.0.0";
    }
};

TEST(Messages, Messages)
{
    Messages messages;
    Message1 m1;
    Message2 m2;
    Message3 m3;
    std::unique_ptr<IMessage> m1p = std::make_unique<Message1> (m1);
    std::unique_ptr<IMessage> m2p = std::make_unique<Message2> (m2);
    std::unique_ptr<IMessage> m3p = std::make_unique<Message3> (m3);
    EXPECT_TRUE(messages.empty());
    EXPECT_NO_THROW(messages.add(m1p));
    EXPECT_NO_THROW(messages.add(m2p));
    EXPECT_NO_THROW(messages.add(m3p));
    EXPECT_FALSE(messages.empty());
    EXPECT_EQ(messages.size(), 3);

    Messages mCopy(messages);
    EXPECT_TRUE(mCopy.contains(m1p));
    EXPECT_TRUE(mCopy.contains(m1.getMessageType()));
    EXPECT_TRUE(mCopy.contains(m2p));
    EXPECT_TRUE(mCopy.contains(m3p));

    EXPECT_NO_THROW(mCopy.remove(m2p));
    EXPECT_EQ(mCopy.size(), 2);
    EXPECT_TRUE(mCopy.contains(m1p));
    EXPECT_FALSE(mCopy.contains(m2p));
    EXPECT_TRUE(mCopy.contains(m3p));


    messages.clear();
    EXPECT_EQ(messages.size(), 0);
}

}
