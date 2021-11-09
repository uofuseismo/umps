#include <iostream>
#include <string>
#include <vector>
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Messaging::Authentication;

TEST(ConnectionInformation, Details)
{
    const std::string moduleName = "testModule";
    const std::string connectionString = "tcp://127.0.0.1:8080";
    auto connectionType = ConnectionType::BROADCAST;
    auto securityLevel = UAuth::SecurityLevel::GRASSLANDS;
    auto privileges = UAuth::UserPrivileges::READ_WRITE;

    Details details;

    SocketDetails::Publisher pubSocket;
    pubSocket.setAddress(connectionString);

    EXPECT_NO_THROW(details.setName(moduleName));
    //EXPECT_NO_THROW(details.setConnectionString(connectionString));
    EXPECT_NO_THROW(details.setSocketDetails(pubSocket));

    details.setConnectionType(connectionType);
    details.setSecurityLevel(securityLevel);
    details.setUserPrivileges(privileges);

 
    Details detailsCopy(details);
    EXPECT_EQ(detailsCopy.getName(), moduleName);
    //EXPECT_EQ(detailsCopy.getConnectionString(), connectionString);
    EXPECT_EQ(detailsCopy.getConnectionType(), connectionType);
    EXPECT_EQ(detailsCopy.getSecurityLevel(), securityLevel);
    EXPECT_EQ(detailsCopy.getUserPrivileges(), privileges);
    EXPECT_EQ(detailsCopy.getSocketType(), SocketType::PUBLISHER);
    
    auto pubSocketCopy = detailsCopy.getPublisherSocketDetails();
    EXPECT_EQ(pubSocketCopy.getAddress(), connectionString);

    details.clear();
    EXPECT_FALSE(details.haveName());
    //EXPECT_FALSE(details.haveConnectionString());
    EXPECT_FALSE(details.haveConnectionType());
    EXPECT_EQ(details.getSocketType(), SocketType::UNKNOWN);
}

TEST(ConnectionInformation, AvailableConnectionsRequest)
{
    AvailableConnectionsRequest request;
    EXPECT_EQ(request.getMessageType(), 
           "UMPS::Services::ConnectionInformation::AvailableConnectionsRequest");

    auto msg = request.toMessage();
 
    AvailableConnectionsRequest requestCopy;
    requestCopy.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(request.getMessageType(), requestCopy.getMessageType());
}

TEST(ConnectionInformation, AvailableConnectionsResponse)
{
    const ReturnCode returnCode = ReturnCode::ALGORITHM_FAILURE;
    AvailableConnectionsResponse response;
    EXPECT_EQ(response.getMessageType(), 
           "UMPS::Services::ConnectionInformation::AvailableConnectionsResponse");
    response.setReturnCode(returnCode);
    EXPECT_EQ(response.getReturnCode(), returnCode);

    std::vector<std::string> names{"Test1", "Test2"};
    std::vector<std::string> connectionStrings{"tcp://127.0.0.1:5050",
                                               "tcp://127.0.0.1:5060"};
    std::vector<ConnectionType> connectionTypes{ConnectionType::BROADCAST,
                                                ConnectionType::SERVICE};
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
        SocketDetails::Publisher pubSocket;
        pubSocket.setAddress(connectionStrings[i]);
        //detail.setConnectionString(connectionStrings[i]);
        detail.setConnectionType(connectionTypes[i]);
        detail.setUserPrivileges(privileges[i]);
        detail.setSecurityLevel(securityLevels[i]);
        detail.setSocketDetails(pubSocket);
        details.push_back(detail);
    }
    EXPECT_NO_THROW(response.setDetails(details));

    response.setReturnCode(ReturnCode::SUCCESS);
 
    auto message = response.toMessage();
 
    AvailableConnectionsResponse responseCopy;
    responseCopy.fromMessage(message.data(), message.size());
 
    EXPECT_EQ(responseCopy.getMessageType(), response.getMessageType());
    EXPECT_EQ(responseCopy.getReturnCode(),  response.getReturnCode());
    auto detailsCopy = responseCopy.getDetails();
    EXPECT_EQ(detailsCopy.size(), details.size());
    for (int i = 0; i < static_cast<int> (details.size()); ++i)
    {
        EXPECT_EQ(detailsCopy[i].getName(), details[i].getName());
        //EXPECT_EQ(detailsCopy[i].getConnectionString(),
        //          details[i].getConnectionString());
        EXPECT_EQ(detailsCopy[i].getConnectionType(),
                  details[i].getConnectionType());
        EXPECT_EQ(detailsCopy[i].getUserPrivileges(),
                  details[i].getUserPrivileges());
        EXPECT_EQ(detailsCopy[i].getSecurityLevel(),
                  details[i].getSecurityLevel());
    }
}

}
