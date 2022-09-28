#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include "umps/proxyServices/incrementer/incrementResponse.hpp"
#include "umps/proxyServices/incrementer/incrementRequest.hpp"
#include "umps/proxyServices/incrementer/itemsRequest.hpp"
#include "umps/proxyServices/incrementer/itemsResponse.hpp"
#include "umps/proxyServices/incrementer/replierOptions.hpp"
#include "umps/proxyServices/incrementer/requestorOptions.hpp"
#include "umps/proxyServices/incrementer/counter.hpp"
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>
namespace
{
namespace UMPSIC = UMPS::ProxyServices::Incrementer;
namespace UAuth = UMPS::Authentication;

TEST(Incrementer, Options)
{
/*
    const char *cParms = "[Counters]\nsqlite3FileName = tables/counter.sqlite3\ninitialValue = 2\nincrement = 3\nbackendAddress = tcp://localhost:5560\nclientAccessAddress = tcp://localhost:5559\nverbosity = 2\n";
    //std::cout << cParms << std::endl;
    const std::string iniFileName = "incrementerExample.ini";
    std::ofstream tempFile(iniFileName);
    tempFile << cParms;
    tempFile.close();
*/
    UMPSIC::Options options; 
    options.setSqlite3FileName("tables/counter.sqlite3");
    options.setBackendAddress("tcp://localhost:5560");
    //options.setVerbosity(UMPS::Logging::Level::DEBUG);
    options.setInitialValue(42);
    options.setIncrement(84);

    //EXPECT_NO_THROW(
    //    options.parseInitializationFile(iniFileName, "Counters"));
    //std::remove(iniFileName.c_str());
    EXPECT_EQ(options.getSqlite3FileName(), "tables/counter.sqlite3");
    EXPECT_EQ(options.getInitialValue(), 42);
    EXPECT_EQ(options.getIncrement(), 84);
    EXPECT_EQ(options.getBackendAddress(), "tcp://localhost:5560");
    //EXPECT_EQ(options.getVerbosity(), UMPS::Logging::Level::DEBUG); //static_cast<UMPS::Logging::Level> (2));
}

TEST(Incrementer, RequestOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const int hwm = 100;
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    UMPSIC::RequestorOptions options;
    options.setAddress(frontendAddress);
    options.setHighWaterMark(hwm);
    options.setZAPOptions(zapOptions);

    UMPSIC::RequestorOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), frontendAddress);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), hwm);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    auto messages = optionsCopy.getRequestOptions().getMessageFormats();
    UMPSIC::ItemsResponse itemsResponse;
    UMPSIC::IncrementResponse incrementResponse;
    EXPECT_TRUE(messages.contains(itemsResponse.getMessageType()));
    EXPECT_TRUE(messages.contains(incrementResponse.getMessageType()));

    

    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), 2048);//1024);
}

TEST(Incrementer, ReplierOptions)
{
    const std::string address = "tcp://localhost:5050";
    const int hwm = 1234;
    UMPSIC::ReplierOptions replier;
    replier.setAddress(address);
    replier.setHighWaterMark(hwm);
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    replier.setZAPOptions(zapOptions);

    UMPSIC::ReplierOptions optionsCopy(replier);
    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    EXPECT_EQ(optionsCopy.getHighWaterMark(), hwm);
}

TEST(Incrementer, IncrementRequest)
{
    UMPSIC::IncrementRequest request;
    uint64_t id = 553;
    const std::string item = "abc";

    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::PHASE_PICK));
    EXPECT_EQ(request.getItem(), "PhasePick");
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::PHASE_ARRIVAL));
    EXPECT_EQ(request.getItem(), "PhaseArrival");
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::EVENT));
    EXPECT_EQ(request.getItem(), "Event");
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::ORIGIN));
    EXPECT_EQ(request.getItem(), "Origin");
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::AMPLITUDE));
    EXPECT_EQ(request.getItem(), "Amplitude");
    EXPECT_NO_THROW(request.setItem(UMPSIC::Item::MAGNITUDE));
    EXPECT_EQ(request.getItem(), "Magnitude");

    EXPECT_NO_THROW(request.setItem(item));
    request.setIdentifier(id);
    EXPECT_EQ(request.getIdentifier(), id);

    auto msg = request.toMessage();
    UMPSIC::IncrementRequest rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(msg)); //msg.data(), msg.size());
    EXPECT_EQ(rCopy.getItem(), item);
    EXPECT_EQ(rCopy.getIdentifier(), id);

    request.clear();
    EXPECT_FALSE(request.haveItem()); 
    EXPECT_EQ(request.getMessageType(), "UMPS::ProxyServices::Incrementer::IncrementRequest");
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
    EXPECT_NO_THROW(rCopy.fromMessage(msg)); //msg.data(), msg.size());
    EXPECT_EQ(rCopy.getIdentifier(), id);

    request.clear();
    EXPECT_EQ(request.getMessageType(), "UMPS::ProxyServices::Incrementer::ItemsRequest");
}

TEST(Incrementer, ItemsResponse)
{
    UMPSIC::ItemsResponse response;
    uint64_t id = 3423;
    const std::set<std::string> items{"abc", "123", "mj"};
    
    response.setIdentifier(id);
    EXPECT_NO_THROW(response.setItems(items));

    auto msg = response.toMessage();
    UMPSIC::ItemsResponse rCopy;
    EXPECT_NO_THROW(rCopy.fromMessage(msg)); //msg.data(), msg.size());
    EXPECT_EQ(rCopy.getItems().size(), items.size());
    EXPECT_EQ(rCopy.getItems(), items);
    EXPECT_EQ(rCopy.getIdentifier(), id);
 
    response.clear();
    EXPECT_EQ(response.getMessageType(), "UMPS::ProxyServices::Incrementer::ItemsResponse");
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
    EXPECT_NO_THROW(rCopy.fromMessage(msg)); //msg.data(), msg.size());
    EXPECT_EQ(rCopy.getValue(), value);
    EXPECT_EQ(rCopy.getIdentifier(), id);
    EXPECT_EQ(rCopy.getReturnCode(), code);

    EXPECT_EQ(response.getMessageType(),
              "UMPS::ProxyServices::Incrementer::IncrementResponse");
}

TEST(Incrementer, Counter)
{
    UMPSIC::Counter counter;
    std::string tableName = "tables/counter.sqlite3";
    bool deleteIfExists = true;
    const std::string item = "test";
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

