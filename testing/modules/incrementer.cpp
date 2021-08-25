#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include "urts/modules/incrementer/response.hpp"
#include "urts/modules/incrementer/request.hpp"
#include "urts/modules/incrementer/counter.hpp"
#include <gtest/gtest.h>
namespace
{
namespace URTSIC = URTS::Modules::Incrementer;

TEST(Incrementer, Request)
{
    URTSIC::Request request;
    uint64_t id = 553;
    const std::string item = "abc";

    EXPECT_NO_THROW(request.setItem(URTSIC::Item::PHASE_PICK));
    EXPECT_NO_THROW(request.setItem(URTSIC::Item::PHASE_ARRIVAL));
    EXPECT_NO_THROW(request.setItem(URTSIC::Item::EVENT));
    EXPECT_NO_THROW(request.setItem(URTSIC::Item::ORIGIN));
    EXPECT_NO_THROW(request.setItem(item));
    request.setIdentifier(id);
    EXPECT_EQ(request.getIdentifier(), id);

    auto msg = request.toCBOR();
    URTSIC::Request rCopy;
    rCopy.fromCBOR(msg);
    EXPECT_EQ(rCopy.getItem(), item);
    EXPECT_EQ(rCopy.getIdentifier(), id);

    request.clear();
    EXPECT_FALSE(request.haveItem()); 
    EXPECT_EQ(request.getMessageType(), "URTS::Modules::Incrementer::Request");
}

TEST(Incrementer, Response)
{
    URTSIC::Response response;
    uint64_t id = 553;
    uint64_t value = 3938;
    auto code = URTSIC::ReturnCode::NO_ITEM;
    EXPECT_FALSE(response.haveValue());
    response.setValue(value);
    response.setIdentifier(id);
    EXPECT_TRUE(response.haveValue());
    EXPECT_EQ(response.getReturnCode(), URTSIC::ReturnCode::SUCCESS);
    response.setReturnCode(code);     
 
    auto msg = response.toCBOR();

    URTSIC::Response rCopy;
    rCopy.fromCBOR(msg);
    EXPECT_EQ(rCopy.getValue(), value);
    EXPECT_EQ(rCopy.getIdentifier(), id);
    EXPECT_EQ(rCopy.getReturnCode(), code);

    EXPECT_EQ(response.getMessageType(),
              "URTS::Modules::Incrementer::Response");
}

TEST(Incrementer, Counter)
{
    URTSIC::Counter counter;
    const std::string item = "test";
    const uint64_t initialCount = 5;
    const uint64_t interval = 10;
    EXPECT_NO_THROW(counter.initialize(item, initialCount, interval)); 
    EXPECT_EQ(counter.getInitialValue(), initialCount);
    EXPECT_EQ(counter.getName(), item);
    EXPECT_EQ(counter.getIncrement(), interval);
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(counter.getCurrentValue(), initialCount + i*interval);
        EXPECT_EQ(counter.getNextValue(), initialCount + (i + 1)*interval);
    }
    EXPECT_NO_THROW(counter.reset());
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(counter.getCurrentValue(), initialCount + i*interval);
        EXPECT_EQ(counter.getNextValue(), initialCount + (i + 1)*interval);
    }
    counter.clear();
    EXPECT_FALSE(counter.isInitialized());
}

}

