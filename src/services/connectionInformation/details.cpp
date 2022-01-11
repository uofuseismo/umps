#include <string>
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/subscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/publisher.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Details::DetailsImpl
{
public:
    std::string mName;
    std::string mConnectionString;
    ConnectionType mConnectionType;
    SocketDetails::Dealer mDealer;
    SocketDetails::Proxy mProxy;
    SocketDetails::Publisher mPublisher;
    SocketDetails::Request mRequest;
    SocketDetails::Router mRouter;
    SocketDetails::Subscriber mSubscriber;
    SocketDetails::XPublisher mXPublisher;
    SocketDetails::XSubscriber mXSubscriber;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS; 
    UAuth::UserPrivileges mUserPrivileges = UAuth::UserPrivileges::READ_ONLY;
    SocketType mSocketType = SocketType::UNKNOWN;
    bool mHaveConnectionType = false;
};

/// C'tor
Details::Details() :
    pImpl(std::make_unique<DetailsImpl> ())
{
}

/// Copy c'tor
Details::Details(const Details &details)
{
    *this = details;
}

/// Move c'tor
Details::Details(Details &&details) noexcept
{
    *this = std::move(details);
}

/// Copy assignment
Details& Details::operator=(const Details &details)
{
    if (&details == this){return *this;}
    pImpl = std::make_unique<DetailsImpl> (*details.pImpl);
    return *this;
}

/// Move assignment
Details& Details::operator=(Details &&details) noexcept
{
    if (&details == this){return *this;}
    pImpl = std::move(details.pImpl);
    return *this;
}

/// Destructor
Details::~Details() = default;

/// Reset class
void Details::clear() noexcept
{
    pImpl = std::make_unique<DetailsImpl> ();
}

/// Name
void Details::setName(const std::string &name)
{
    if (isEmpty(name))
    {
        throw std::invalid_argument("Name is empty");
    }
    pImpl->mName = name;
}

std::string Details::getName() const
{
    if (!haveName()){throw std::runtime_error("Name not set");}
    return pImpl->mName;
}

bool Details::haveName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Connection string
/*
void Details::setConnectionString(const std::string &connectionString)
{
    if (isEmpty(connectionString))
    {
        throw std::invalid_argument("Connection string is empty");
    }
    pImpl->mConnectionString = connectionString;
}

std::string Details::getConnectionString() const
{
    if (!haveConnectionString())
    {
        throw std::runtime_error("Connection string not defined");
    }
    return pImpl->mConnectionString;
}

bool Details::haveConnectionString() const noexcept
{
    return !pImpl->mConnectionString.empty();
}
*/

/// Security level
void Details::setSecurityLevel(UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel Details::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}

/// Privileges
void Details::setUserPrivileges(UAuth::UserPrivileges privileges) noexcept
{
    pImpl->mUserPrivileges = privileges;
}

UAuth::UserPrivileges Details::getUserPrivileges() const noexcept
{
    return pImpl->mUserPrivileges;
}

/// Connection type
void Details::setConnectionType(const ConnectionType connectionType) noexcept
{
    pImpl->mConnectionType = connectionType;
    pImpl->mHaveConnectionType = true;
}

ConnectionType Details::getConnectionType() const
{
    if (!haveConnectionType())
    {
        throw std::runtime_error("Connection type not set");
    }
    return pImpl->mConnectionType;
}

bool Details::haveConnectionType() const noexcept
{
    return pImpl->mHaveConnectionType;
}

/// Set socket details
void Details::setSocketDetails(const SocketDetails::Dealer &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mDealer = socket;
    pImpl->mSocketType = socket.getSocketType();
}

void Details::setSocketDetails(const SocketDetails::Publisher &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mPublisher = socket;
    pImpl->mSocketType = socket.getSocketType();
}

void Details::setSocketDetails(const SocketDetails::Subscriber &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mSubscriber = socket;
    pImpl->mSocketType = socket.getSocketType();
}

void Details::setSocketDetails(const SocketDetails::Request &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mRequest = socket;
    pImpl->mSocketType = socket.getSocketType();
}

void Details::setSocketDetails(const SocketDetails::Router &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mRouter = socket;
    pImpl->mSocketType = socket.getSocketType();
}

void Details::setSocketDetails(const SocketDetails::XPublisher &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mXPublisher = socket;
    pImpl->mSocketType = socket.getSocketType();
}
    
void Details::setSocketDetails(const SocketDetails::XSubscriber &socket)
{
    if (!socket.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mXSubscriber = socket; 
    pImpl->mSocketType = socket.getSocketType();
}

void Details::setSocketDetails(const SocketDetails::Proxy &socket)
{
    if (!socket.haveSocketPair())
    {
        throw std::invalid_argument("Socket pair not set");
    }
    pImpl->mProxy = socket;
    pImpl->mSocketType = socket.getSocketType();
}

/// Get socket details
SocketDetails::Dealer Details::getDealerSocketDetails() const
{
    if (getSocketType() != SocketType::DEALER)
    {
        throw std::runtime_error("Dealer socket details not set");
    }
    return pImpl->mDealer;
}

SocketDetails::Publisher Details::getPublisherSocketDetails() const
{
    if (getSocketType() != SocketType::PUBLISHER)
    {
        throw std::runtime_error("Publisher socket details not set");
    }
    return pImpl->mPublisher;
}

SocketDetails::Subscriber Details::getSubscriberSocketDetails() const
{
    if (getSocketType() != SocketType::SUBSCRIBER)
    {
        throw std::runtime_error("Subscriber socket details not set");
    }
    return pImpl->mSubscriber;
}

SocketDetails::Request Details::getRequestSocketDetails() const
{
    if (getSocketType() != SocketType::REQUEST)
    {
        throw std::runtime_error("Request socket details not set");
    }
    return pImpl->mRequest;
}

SocketDetails::Router Details::getRouterSocketDetails() const
{
    if (getSocketType() != SocketType::ROUTER)
    {
        throw std::runtime_error("Router socket details not set");
    }
    return pImpl->mRouter;
}

SocketDetails::XPublisher Details::getXPublisherSocketDetails() const
{
    if (getSocketType() != SocketType::XPUBLISHER)
    {
        throw std::runtime_error("XPublisher socket details not set");
    }
    return pImpl->mXPublisher;
}

SocketDetails::XSubscriber Details::getXSubscriberSocketDetails() const
{
    if (getSocketType() != SocketType::XSUBSCRIBER)
    {
        throw std::runtime_error("XSubscriber socket details not set");
    }
    return pImpl->mXSubscriber;
}

SocketDetails::Proxy Details::getProxySocketDetails() const
{
    if (getSocketType() != SocketType::PROXY)
    {
        throw std::runtime_error("Proxy socket details not set");
    }
    return pImpl->mProxy;
}

SocketType Details::getSocketType() const noexcept
{
    return pImpl->mSocketType;
}
