#include <iostream>
#include <string>
#include <vector>
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

TEST(Messaging, RequestorOptions)
{
    RequestorOptions options;
    const std::string address = "tcp://127.0.0.2:5556";
    const std::chrono::milliseconds timeOut{120};
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    EXPECT_NO_THROW(options.setZAPOptions(zapOptions));
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setTimeOut(timeOut));
    
    RequestorOptions optionsCopy(options); 
    auto optionsBase = optionsCopy.getRequestOptions();
    EXPECT_EQ(optionsBase.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Strawhouse);
    EXPECT_EQ(optionsBase.getAddress(), address);
    EXPECT_EQ(optionsBase.getTimeOut(), timeOut);

    options.clear();
    optionsBase = options.getRequestOptions();
    EXPECT_EQ(optionsBase.getTimeOut(), std::chrono::seconds{5});
}

TEST(ConnectionInformation, SocketDetails)
{
    const std::string frontEnd = "tcp://127.0.0.1:8080";
    const std::string backEnd  = "tcp://127.0.0.1:8081";

    SocketDetails::Publisher publisher;
    EXPECT_NO_THROW(publisher.setAddress(frontEnd));
    publisher.setConnectOrBind(ConnectOrBind::Bind);
    EXPECT_EQ(publisher.getAddress(), frontEnd);
    EXPECT_EQ(publisher.getSocketType(), SocketType::Publisher);
    EXPECT_EQ(publisher.getConnectOrBind(), ConnectOrBind::Bind);

    SocketDetails::Subscriber subscriber;
    EXPECT_NO_THROW(subscriber.setAddress(frontEnd));
    subscriber.setConnectOrBind(ConnectOrBind::Bind);
    EXPECT_EQ(subscriber.getAddress(), frontEnd);
    EXPECT_EQ(subscriber.getSocketType(), SocketType::Subscriber);
    EXPECT_EQ(subscriber.getConnectOrBind(), ConnectOrBind::Bind);

    SocketDetails::XPublisher xPublisher;
    EXPECT_NO_THROW(xPublisher.setAddress(backEnd));
    xPublisher.setConnectOrBind(ConnectOrBind::Connect);
    EXPECT_EQ(xPublisher.getAddress(), backEnd);
    EXPECT_EQ(xPublisher.getSocketType(), SocketType::XPublisher);
    EXPECT_EQ(xPublisher.getConnectOrBind(), ConnectOrBind::Connect);

    SocketDetails::XSubscriber xSubscriber;
    EXPECT_NO_THROW(xSubscriber.setAddress(frontEnd));
    xSubscriber.setConnectOrBind(ConnectOrBind::Connect);
    EXPECT_EQ(xSubscriber.getAddress(), frontEnd);
    EXPECT_EQ(xSubscriber.getSocketType(), SocketType::XSubscriber);
    EXPECT_EQ(xSubscriber.getConnectOrBind(), ConnectOrBind::Connect);

    SocketDetails::Router router;
    EXPECT_NO_THROW(router.setAddress(frontEnd));
    router.setConnectOrBind(ConnectOrBind::Connect);
    EXPECT_EQ(router.getAddress(), frontEnd);
    EXPECT_EQ(router.getSocketType(), SocketType::Router);
    EXPECT_EQ(router.getConnectOrBind(), ConnectOrBind::Connect);

    SocketDetails::Request request;
    EXPECT_NO_THROW(request.setAddress(frontEnd));
    request.setConnectOrBind(ConnectOrBind::Bind);
    EXPECT_EQ(request.getAddress(), frontEnd);
    EXPECT_EQ(request.getSocketType(), SocketType::Request);
    EXPECT_EQ(request.getConnectOrBind(), ConnectOrBind::Bind);

    SocketDetails::Reply reply;
    EXPECT_NO_THROW(reply.setAddress(backEnd));
    reply.setConnectOrBind(ConnectOrBind::Connect);
    EXPECT_EQ(reply.getAddress(), backEnd);
    EXPECT_EQ(reply.getSocketType(), SocketType::Reply);
    EXPECT_EQ(reply.getConnectOrBind(), ConnectOrBind::Connect);

    SocketDetails::Dealer dealer;
    EXPECT_NO_THROW(dealer.setAddress(backEnd));
    dealer.setConnectOrBind(ConnectOrBind::Connect);
    EXPECT_EQ(dealer.getAddress(), backEnd);
    EXPECT_EQ(dealer.getSocketType(), SocketType::Dealer);
    EXPECT_EQ(dealer.getConnectOrBind(), ConnectOrBind::Connect);

    SocketDetails::Proxy proxy;
    EXPECT_NO_THROW(proxy.setSocketPair(std::pair(xSubscriber, xPublisher)));
    EXPECT_EQ(proxy.getSocketType(), SocketType::Proxy);
    EXPECT_EQ(proxy.getFrontendSocketType(), SocketType::XSubscriber);
    EXPECT_EQ(proxy.getBackendSocketType(),  SocketType::XPublisher);
    auto xSubCopy = proxy.getXSubscriberFrontend();
    auto xPubCopy = proxy.getXPublisherBackend(); 
    EXPECT_EQ(xSubCopy.getAddress(), frontEnd);
    EXPECT_EQ(xSubCopy.getSocketType(), SocketType::XSubscriber);
    EXPECT_EQ(xSubCopy.getConnectOrBind(), ConnectOrBind::Connect);
    EXPECT_EQ(xPubCopy.getAddress(), backEnd);
    EXPECT_EQ(xPubCopy.getSocketType(), SocketType::XPublisher);
    EXPECT_EQ(xPubCopy.getConnectOrBind(), ConnectOrBind::Connect);

    EXPECT_NO_THROW(proxy.setSocketPair(std::pair(router, dealer)));
    EXPECT_EQ(proxy.getSocketType(), SocketType::Proxy);
    EXPECT_EQ(proxy.getFrontendSocketType(), SocketType::Router);
    EXPECT_EQ(proxy.getBackendSocketType(),  SocketType::Dealer);
    auto routerCopy = proxy.getRouterFrontend();
    auto dealerCopy = proxy.getDealerBackend(); 
    EXPECT_EQ(routerCopy.getAddress(), frontEnd);
    EXPECT_EQ(routerCopy.getSocketType(), SocketType::Router);
    EXPECT_EQ(routerCopy.getConnectOrBind(), ConnectOrBind::Connect);
    EXPECT_EQ(dealerCopy.getAddress(), backEnd);
    EXPECT_EQ(dealerCopy.getSocketType(), SocketType::Dealer);
    EXPECT_EQ(dealerCopy.getConnectOrBind(), ConnectOrBind::Connect);
}

TEST(ConnectionInformation, Details)
{
    const std::string moduleName = "testModule";
    const std::string connectionString = "tcp://127.0.0.1:8080";
    auto connectionType = ConnectionType::Broadcast;
    auto securityLevel = UAuth::SecurityLevel::Grasslands;
    //auto privileges = UAuth::UserPrivileges::ReadWrite;

    Details details;

    SocketDetails::Publisher pubSocket;
    pubSocket.setAddress(connectionString);

    EXPECT_NO_THROW(details.setName(moduleName));
    //EXPECT_NO_THROW(details.setConnectionString(connectionString));
    EXPECT_NO_THROW(details.setSocketDetails(pubSocket));

    details.setConnectionType(connectionType);
    details.setSecurityLevel(securityLevel);
    //details.setUserPrivileges(privileges);

 
    Details detailsCopy(details);
    EXPECT_EQ(detailsCopy.getName(), moduleName);
    //EXPECT_EQ(detailsCopy.getConnectionString(), connectionString);
    EXPECT_EQ(detailsCopy.getConnectionType(), connectionType);
    EXPECT_EQ(detailsCopy.getSecurityLevel(), securityLevel);
    //EXPECT_EQ(detailsCopy.getUserPrivileges(), privileges);
    EXPECT_EQ(detailsCopy.getSocketType(), SocketType::Publisher);
    
    auto pubSocketCopy = detailsCopy.getPublisherSocketDetails();
    EXPECT_EQ(pubSocketCopy.getAddress(), connectionString);

    details.clear();
    EXPECT_FALSE(details.haveName());
    //EXPECT_FALSE(details.haveConnectionString());
    EXPECT_FALSE(details.haveConnectionType());
    EXPECT_EQ(details.getSocketType(), SocketType::Unknown);
}

TEST(ConnectionInformation, AvailableConnectionsRequest)
{
    AvailableConnectionsRequest request;
    EXPECT_EQ(request.getMessageType(), 
           "UMPS::Services::ConnectionInformation::AvailableConnectionsRequest");

    auto msg = request.toMessage();
 
    AvailableConnectionsRequest requestCopy;
    EXPECT_NO_THROW(requestCopy.fromMessage(msg)); //msg.data(), msg.size());
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
    std::vector<ConnectionType> connectionTypes{ConnectionType::Broadcast,
                                                ConnectionType::Service};
    std::vector<UAuth::UserPrivileges>
        privileges{UAuth::UserPrivileges::ReadWrite,
                   UAuth::UserPrivileges::Administrator};
    std::vector<UAuth::SecurityLevel>
        securityLevels{UAuth::SecurityLevel::Stonehouse,
                       UAuth::SecurityLevel::Woodhouse};

    std::vector<Details> details;
    for (int i = 0; i < static_cast<int> (names.size()); ++i)
    {
        Details detail;
        detail.setName(names[i]);
        SocketDetails::Publisher pubSocket;
        pubSocket.setAddress(connectionStrings[i]);
        pubSocket.setMinimumUserPrivileges(privileges[i]);
        pubSocket.setSecurityLevel(securityLevels[i]);
        //detail.setConnectionString(connectionStrings[i]);
        detail.setConnectionType(connectionTypes[i]);
        //detail.setUserPrivileges(privileges[i]);
        detail.setSecurityLevel(securityLevels[i]);
        detail.setSocketDetails(pubSocket);
        details.push_back(detail);
    }
    EXPECT_NO_THROW(response.setDetails(details));

    response.setReturnCode(ReturnCode::SUCCESS);
 
    auto message = response.toMessage();
 
    AvailableConnectionsResponse responseCopy;
    EXPECT_NO_THROW(responseCopy.fromMessage(message)); //message.data(), message.size());
 
    EXPECT_EQ(responseCopy.getMessageType(), response.getMessageType());
    EXPECT_EQ(responseCopy.getReturnCode(),  response.getReturnCode());
    auto detailsCopy = responseCopy.getDetails();
    EXPECT_EQ(detailsCopy.size(), details.size());
    for (int i = 0; i < static_cast<int> (details.size()); ++i)
    {
        auto pubSocket = detailsCopy[i].getPublisherSocketDetails();
        EXPECT_EQ(pubSocket.getAddress(), connectionStrings[i]);
        EXPECT_EQ(pubSocket.getMinimumUserPrivileges(), privileges[i]);
        EXPECT_EQ(pubSocket.getSecurityLevel(), securityLevels[i]);

        EXPECT_EQ(detailsCopy[i].getName(), details[i].getName());
        //EXPECT_EQ(detailsCopy[i].getConnectionString(),
        //          details[i].getConnectionString());
        EXPECT_EQ(detailsCopy[i].getConnectionType(),
                  details[i].getConnectionType());
        //EXPECT_EQ(detailsCopy[i].getUserPrivileges(),
        //          details[i].getUserPrivileges());
        EXPECT_EQ(detailsCopy[i].getSecurityLevel(),
                  details[i].getSecurityLevel());
    }
}

}
