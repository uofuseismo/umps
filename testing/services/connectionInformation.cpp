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

TEST(ConnectionInformation, Details)
{
    const std::string moduleName = "testModule";
    const std::string connectionString = "tcp://127.0.0.1:8080";
    auto securityLevel
      = UMPS::Messaging::Authentication::SecurityLevel::GRASSLANDS;
    auto privileges
      = UMPS::Messaging::Authentication::UserPrivileges::READ_WRITE;

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

}
