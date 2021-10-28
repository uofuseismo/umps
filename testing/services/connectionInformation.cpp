#include <iostream>
#include <string>
#include <vector>
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/availableBroadcastsRequest.hpp"
#include "umps/services/connectionInformation/availableBroadcastsResponse.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Messaging::Authentication;

TEST(ConnectionInformation, Details)
{
    const std::string moduleName = "testModule";
    const std::string connectionString = "tcp://127.0.0.1:8080";
    auto securityLevel = UAuth::SecurityLevel::GRASSLANDS;
    auto privileges = UAuth::UserPrivileges::READ_WRITE;

    Details details;

    EXPECT_NO_THROW(details.setName(moduleName));
    EXPECT_NO_THROW(details.setConnectionString(connectionString));
    details.setSecurityLevel(securityLevel);
    details.setUserPrivileges(privileges);

 
    Details detailsCopy(details);
    EXPECT_EQ(detailsCopy.getName(), moduleName);
    EXPECT_EQ(detailsCopy.getConnectionString(), connectionString);
    EXPECT_EQ(detailsCopy.getSecurityLevel(), securityLevel);
    EXPECT_EQ(detailsCopy.getUserPrivileges(), privileges);
    

    details.clear();
    EXPECT_FALSE(details.haveName());
    EXPECT_FALSE(details.haveConnectionString());
}

TEST(ConnectionInformation, AvailableBroadcastsRequest)
{
    AvailableBroadcastsRequest request;
    EXPECT_EQ(request.getMessageType(), 
           "UMPS::Services::ConnectionInformation::AvailableBroadcastsRequest");

    auto msg = request.toMessage();
 
    AvailableBroadcastsRequest requestCopy;
    requestCopy.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(request.getMessageType(), requestCopy.getMessageType());
}

TEST(ConnectionInformation, AvailableBroadcastsResponse)
{
    const ReturnCode returnCode = ReturnCode::ALGORITHM_FAILURE;
    AvailableBroadcastsResponse response;
    response.setReturnCode(returnCode);
    EXPECT_EQ(response.getReturnCode(), returnCode);

    std::vector<std::string> names{"Test1", "Test2"};
    std::vector<std::string> connectionStrings{"tcp://127.0.0.1:5050",
                                               "tcp://127.0.0.1:5060"};
    std::vector<UAuth::UserPrivileges>
        privileges{UAuth::UserPrivileges::READ_WRITE,
                   UAuth::UserPrivileges::ADMINISTRATOR};
    std::vector<UAuth::SecurityLevel>
        securityLevels{UAuth::SecurityLevel::STONEHOUSE,
                       UAuth::SecurityLevel::WOODHOUSE};

    std::vector<Details> details;
    for (int i = 0; i < static_cast<int> (names.size()); ++i)
    {
        Details detail;
        detail.setName(names[i]);
        detail.setConnectionString(connectionStrings[i]);
        detail.setUserPrivileges(privileges[i]);
        detail.setSecurityLevel(securityLevels[i]);
        details.push_back(detail);
    }
    EXPECT_NO_THROW(response.setDetails(details));

    response.setReturnCode(ReturnCode::SUCCESS);
 
    auto message = response.toMessage();
 
    AvailableBroadcastsResponse responseCopy;
    responseCopy.fromMessage(message.data(), message.size());
 
    EXPECT_EQ(responseCopy.getMessageType(), response.getMessageType());
    EXPECT_EQ(responseCopy.getReturnCode(),  response.getReturnCode());
    auto detailsCopy = responseCopy.getDetails();
    EXPECT_EQ(detailsCopy.size(), details.size());
    for (int i = 0; i < static_cast<int> (details.size()); ++i)
    {
        EXPECT_EQ(detailsCopy[i].getName(), details[i].getName());
        EXPECT_EQ(detailsCopy[i].getConnectionString(),
                  details[i].getConnectionString());
        EXPECT_EQ(detailsCopy[i].getUserPrivileges(),
                  details[i].getUserPrivileges());
        EXPECT_EQ(detailsCopy[i].getSecurityLevel(),
                  details[i].getSecurityLevel());
    }
}

}
