#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include "umps/services/incrementer/incrementResponse.hpp"
#include "umps/services/incrementer/incrementRequest.hpp"
#include "umps/services/incrementer/itemsRequest.hpp"
#include "umps/services/incrementer/counter.hpp"
#include "umps/services/incrementer/options.hpp"
#include <gtest/gtest.h>
namespace
{
namespace UMPSIC = UMPS::Services::Incrementer;

TEST(Incrementer, Parameters)
{
    const char *cParms = "[Counters]\nsqlite3FileName = tables/counter.sqlite3\ninitialValue = 2\nincrement = 3\nserverAccessAddress = tcp://localhost:5560\nclientAccessAddress = tcp://localhost:5559\nverbosity = 2\n";
    //std::cout << cParms << std::endl;
    const std::string iniFileName = "incrementerExample.ini";
    std::ofstream tempFile(iniFileName);
    tempFile << cParms;
    tempFile.close();
    UMPSIC::Options parameters;
    EXPECT_NO_THROW(
        parameters.parseInitializationFile(iniFileName, "Counters"));
    std::remove(iniFileName.c_str());
    EXPECT_EQ(parameters.getSqlite3FileName(), "tables/counter.sqlite3");
    EXPECT_EQ(parameters.getInitialValue(), 2);
    EXPECT_EQ(parameters.getIncrement(), 3);
    //EXPECT_EQ(parameters.getServerAccessAddress(), "tcp://localhost:5560");
    EXPECT_EQ(parameters.getClientAccessAddress(), "tcp://localhost:5559");
    EXPECT_EQ(parameters.getVerbosity(), static_cast<UMPS::Logging::Level> (2));
}

TEST(Incrementer, IncrementRequest)
{
    UMPSIC::IncrementRequest request;
    uint64_t id = 553;
    const std::string item = "abc";

    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::PHASE_PICK));
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::PHASE_ARRIVAL));
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::EVENT));
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::ORIGIN));
    EXPECT_NO_THROW(request.setItem(item));
    request.setIdentifier(id);
    EXPECT_EQ(request.getIdentifier(), id);

    auto msg = request.toMessage();
    UMPSIC::IncrementRequest rCopy;
    rCopy.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(rCopy.getItem(), item);
    EXPECT_EQ(rCopy.getIdentifier(), id);

    request.clear();
    EXPECT_FALSE(request.haveItem()); 
    EXPECT_EQ(request.getMessageType(), "UMPS::Services::Incrementer::IncrementRequest");
}

TEST(Incrementer, ItemsRequest)
{
    UMPSIC::ItemsRequest request;
    uint64_t id = 253;
    const std::string item = "def";

    request.setIdentifier(id);
    EXPECT_EQ(request.getIdentifier(), id);

    auto msg = request.toMessage();
    UMPSIC::ItemsRequest rCopy;
    rCopy.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(rCopy.getIdentifier(), id);

    request.clear();
    EXPECT_EQ(request.getMessageType(), "UMPS::Services::Incrementer::ItemsRequest");
}

TEST(Incrementer, IncrementResponse)
{
    UMPSIC::IncrementResponse response;
    uint64_t id = 553;
    int64_t value = 3938;
    auto code = UMPSIC::ReturnCode::NO_ITEM;
    EXPECT_FALSE(response.haveValue());
    response.setValue(value);
    response.setIdentifier(id);
    EXPECT_TRUE(response.haveValue());
    EXPECT_EQ(response.getReturnCode(), UMPSIC::ReturnCode::SUCCESS);
    response.setReturnCode(code);     
 
    auto msg = response.toMessage();

    UMPSIC::IncrementResponse rCopy;
    rCopy.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(rCopy.getValue(), value);
    EXPECT_EQ(rCopy.getIdentifier(), id);
    EXPECT_EQ(rCopy.getReturnCode(), code);

    EXPECT_EQ(response.getMessageType(),
              "UMPS::Services::Incrementer::IncrementResponse");
}

TEST(Incrementer, Counter)
{
    UMPSIC::Counter counter;
    std::string tableName = "tables/counter.sqlite3";
    bool deleteIfExists = true;
    const std::string item = "test";
    const uint64_t initialCount = 5;
    const uint64_t interval = 10;
    std::map<std::string, std::pair<int64_t, int32_t>> items;

    items.insert( std::pair("pick", std::pair(5, 10)) );
    items.insert( std::pair("event", std::pair(0, 1)) );


    EXPECT_NO_THROW(counter.initialize(tableName, items, deleteIfExists));
    EXPECT_TRUE(counter.isInitialized());
    EXPECT_TRUE(counter.haveItem("pick"));
    EXPECT_TRUE(counter.haveItem("event"));

    EXPECT_NO_THROW(counter.addItem("arrival", 4, 11));
    EXPECT_TRUE(counter.haveItem("arrival"));

    auto itemsInTable = counter.getItems();
    EXPECT_EQ(itemsInTable.size(), 3);
    EXPECT_TRUE(itemsInTable.contains("pick")); 
    EXPECT_TRUE(itemsInTable.contains("arrival"));
    EXPECT_TRUE(itemsInTable.contains("event"));

    auto nextValue = counter.getNextValue("pick");
    EXPECT_EQ(nextValue, 5 + 10);
    nextValue = counter.getNextValue("pick");
    EXPECT_EQ(nextValue, 5 + 10 + 10);

    EXPECT_EQ(counter.getCurrentValue("pick"), 5 + 10 + 10);
    EXPECT_EQ(counter.getCurrentValue("arrival"), 4);
    EXPECT_EQ(counter.getCurrentValue("event"), 0);
}

}

