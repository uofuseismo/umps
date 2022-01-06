#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "umps/services/connectionInformation/getConnections.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UM = UMPS::Messaging;
namespace UAuth = UMPS::Authentication;

/// Get the available connections
std::vector<Details>
UMPS::Services::ConnectionInformation::getConnections(
    const std::string &endPoint,
    const UAuth::ZAPOptions &zapOptions)
{
    // Set endpoint
    UM::RequestRouter::RequestOptions requestOptions;
    requestOptions.setEndPoint(endPoint);
    // Set the security options
    requestOptions.setZAPOptions(zapOptions);
    // Tell request types of messages we can unpack 
    std::unique_ptr<UMPS::MessageFormats::IMessage> responseMessage
        = std::make_unique<AvailableConnectionsResponse> (); 
    requestOptions.addMessageFormat(responseMessage);

    // Fire up the client
    UMPS::Messaging::RequestRouter::Request client;
    client.initialize(requestOptions);
    // Get the response
    AvailableConnectionsRequest requestMessage;
    auto message = client.request(requestMessage);
    auto detailsMessage
        = static_unique_pointer_cast<AvailableConnectionsResponse>
          (std::move(message));
    return detailsMessage->getDetails();
}

/// Overloaded function to get the available connections 
std::vector<Details>
UMPS::Services::ConnectionInformation::getConnections(
    const std::string &endPoint)
{
    UAuth::ZAPOptions zapOptions; // Defaults to grasslands
    return getConnections(endPoint, zapOptions);
}
