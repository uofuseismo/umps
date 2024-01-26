#include <python/services.hpp>
#include <python/logging.hpp>
#include <python/messaging.hpp>
#include <umps/messaging/context.hpp>
#include <umps/logging/log.hpp>
#include <umps/services/connectionInformation/enums.hpp>
#include <umps/services/connectionInformation/details.hpp>
#include <umps/services/connectionInformation/requestor.hpp>
#include <umps/services/connectionInformation/requestorOptions.hpp>
#include <umps/services/connectionInformation/availableConnectionsResponse.hpp>
#include <umps/services/connectionInformation/socketDetails/publisher.hpp>
#include <umps/services/connectionInformation/socketDetails/subscriber.hpp>
#include <umps/services/connectionInformation/socketDetails/xPublisher.hpp>
#include <umps/services/connectionInformation/socketDetails/xSubscriber.hpp>
#include <umps/authentication/enums.hpp>

namespace
{
class Subscriber
{
public:
    Subscriber() :
        pImpl(std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::Subscriber> ()) 
    {   
    }
    Subscriber(const Subscriber &details)
    {
        *this = details;
    }
    Subscriber(const UMPS::Services::ConnectionInformation::SocketDetails::Subscriber &details)
    {
        *this = details;
    }
    Subscriber(Subscriber &&details) noexcept
    {
        *this = std::move(details);
    }
    Subscriber& operator=(const Subscriber &details)
    {
        if (&details == this){return *this;}
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::Subscriber> (*details.pImpl);
        return *this;
    }
    Subscriber& operator=(const UMPS::Services::ConnectionInformation::SocketDetails::Subscriber &details)
    {
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::Subscriber> (details);
        return *this;
    }
    Subscriber& operator=(Subscriber &&details) noexcept
    {
        if (&details == this){return *this;}
        pImpl = std::move(details.pImpl);
        return *this;
    }
    UMPS::Services::ConnectionInformation::SocketDetails::Subscriber &getNativeClassReference() const
    {
        return *pImpl;
    }
    void setAddress(const std::string &address)
    {   
        pImpl->setAddress(address);
    }
    std::string getAddress() const
    {
        return pImpl->getAddress();
    }
    void setSecurityLevel(const UMPS::Authentication::SecurityLevel securityLevel) noexcept
    {
        pImpl->setSecurityLevel(securityLevel);
    } 
    void setMinimumUserPrivileges(const UMPS::Authentication::UserPrivileges privileges) noexcept
    {
        return pImpl->setMinimumUserPrivileges(privileges);
    }
    UMPS::Authentication::UserPrivileges getMinimumUserPrivileges() const noexcept
    {
        return pImpl->getMinimumUserPrivileges();
    }
    UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept
    {
        return pImpl->getSecurityLevel();
    }
    UMPS::Services::ConnectionInformation::SocketType getSocketType() const noexcept
    {
        return pImpl->getSocketType();
    } 
    void setConnectOrBind(
        const UMPS::Services::ConnectionInformation::ConnectOrBind connectOrBind) noexcept
    {
        pImpl->setConnectOrBind(connectOrBind);
    }
    UMPS::Services::ConnectionInformation::ConnectOrBind getConnectOrBind() const noexcept
    {
        return pImpl->getConnectOrBind();
    }
    void clear() noexcept
    {
        pImpl->clear();
    }
    ~Subscriber() = default;
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::SocketDetails::Subscriber> pImpl;
};
///----------------------------------XSubscriber-----------------------------///
class XSubscriber
{
public:
    XSubscriber() :
        pImpl(std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber> ()) 
    {   
    }
    XSubscriber(const XSubscriber &details)
    {
        *this = details;
    }
    XSubscriber(const UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber &details)
    {
        *this = details;
    }
    XSubscriber(XSubscriber &&details) noexcept
    {
        *this = std::move(details);
    }
    XSubscriber& operator=(const XSubscriber &details)
    {
        if (&details == this){return *this;}
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber> (*details.pImpl);
        return *this;
    }
    XSubscriber& operator=(const UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber &details)
    {
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber> (details);
        return *this;
    }
    XSubscriber& operator=(XSubscriber &&details) noexcept
    {
        if (&details == this){return *this;}
        pImpl = std::move(details.pImpl);
        return *this;
    }
    UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber &getNativeClassReference() const
    {
        return *pImpl;
    }
    void setAddress(const std::string &address)
    {   
        pImpl->setAddress(address);
    }
    std::string getAddress() const
    {
        return pImpl->getAddress();
    }
    void setSecurityLevel(const UMPS::Authentication::SecurityLevel securityLevel) noexcept
    {
        pImpl->setSecurityLevel(securityLevel);
    } 
    void setMinimumUserPrivileges(const UMPS::Authentication::UserPrivileges privileges) noexcept
    {
        return pImpl->setMinimumUserPrivileges(privileges);
    }
    UMPS::Authentication::UserPrivileges getMinimumUserPrivileges() const noexcept
    {
        return pImpl->getMinimumUserPrivileges();
    }
    UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept
    {
        return pImpl->getSecurityLevel();
    }
    UMPS::Services::ConnectionInformation::SocketType getSocketType() const noexcept
    {
        return pImpl->getSocketType();
    } 
    void setConnectOrBind(
        const UMPS::Services::ConnectionInformation::ConnectOrBind connectOrBind) noexcept
    {
        pImpl->setConnectOrBind(connectOrBind);
    }
    UMPS::Services::ConnectionInformation::ConnectOrBind getConnectOrBind() const noexcept
    {
        return pImpl->getConnectOrBind();
    }
    void clear() noexcept
    {
        pImpl->clear();
    }
    ~XSubscriber() = default;
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::SocketDetails::XSubscriber> pImpl;
};
///----------------------------------Publisher-------------------------------///
class Publisher
{
public:
    Publisher() :
        pImpl(std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::Publisher> ()) 
    {   
    }
    Publisher(const Publisher &details)
    {
        *this = details;
    }
    Publisher(const UMPS::Services::ConnectionInformation::SocketDetails::Publisher &details)
    {
        *this = details;
    }
    Publisher(Publisher &&details) noexcept
    {
        *this = std::move(details);
    }
    Publisher& operator=(const Publisher &details)
    {
        if (&details == this){return *this;}
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::Publisher> (*details.pImpl);
        return *this;
    }
    Publisher& operator=(const UMPS::Services::ConnectionInformation::SocketDetails::Publisher &details)
    {
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::Publisher> (details);
        return *this;
    }
    Publisher& operator=(Publisher &&details) noexcept
    {
        if (&details == this){return *this;}
        pImpl = std::move(details.pImpl);
        return *this;
    }
    UMPS::Services::ConnectionInformation::SocketDetails::Publisher &getNativeClassReference() const
    {
        return *pImpl;
    }
    void setAddress(const std::string &address)
    {   
        pImpl->setAddress(address);
    }
    std::string getAddress() const
    {
        return pImpl->getAddress();
    }
    void setSecurityLevel(const UMPS::Authentication::SecurityLevel securityLevel) noexcept
    {
        pImpl->setSecurityLevel(securityLevel);
    } 
    void setMinimumUserPrivileges(const UMPS::Authentication::UserPrivileges privileges) noexcept
    {
        return pImpl->setMinimumUserPrivileges(privileges);
    }
    UMPS::Authentication::UserPrivileges getMinimumUserPrivileges() const noexcept
    {
        return pImpl->getMinimumUserPrivileges();
    }
    UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept
    {
        return pImpl->getSecurityLevel();
    }
    UMPS::Services::ConnectionInformation::SocketType getSocketType() const noexcept
    {
        return pImpl->getSocketType();
    } 
    void setConnectOrBind(
        const UMPS::Services::ConnectionInformation::ConnectOrBind connectOrBind) noexcept
    {
        pImpl->setConnectOrBind(connectOrBind);
    }
    UMPS::Services::ConnectionInformation::ConnectOrBind getConnectOrBind() const noexcept
    {
        return pImpl->getConnectOrBind();
    }
    void clear() noexcept
    {
        pImpl->clear();
    }
    ~Publisher() = default;
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::SocketDetails::Publisher> pImpl;
};
///----------------------------------XPublisher------------------------------///
class XPublisher
{
public:
    XPublisher() :
        pImpl(std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::XPublisher> ()) 
    {   
    }
    XPublisher(const XPublisher &details)
    {
        *this = details;
    }
    XPublisher(const UMPS::Services::ConnectionInformation::SocketDetails::XPublisher &details)
    {
        *this = details;
    }
    XPublisher(XPublisher &&details) noexcept
    {
        *this = std::move(details);
    }
    XPublisher& operator=(const XPublisher &details)
    {
        if (&details == this){return *this;}
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::XPublisher> (*details.pImpl);
        return *this;
    }
    XPublisher& operator=(const UMPS::Services::ConnectionInformation::SocketDetails::XPublisher &details)
    {
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::SocketDetails::XPublisher> (details);
        return *this;
    }
    XPublisher& operator=(XPublisher &&details) noexcept
    {
        if (&details == this){return *this;}
        pImpl = std::move(details.pImpl);
        return *this;
    }
    UMPS::Services::ConnectionInformation::SocketDetails::XPublisher &getNativeClassReference() const
    {
        return *pImpl;
    }
    void setAddress(const std::string &address)
    {   
        pImpl->setAddress(address);
    }
    std::string getAddress() const
    {
        return pImpl->getAddress();
    }
    void setSecurityLevel(const UMPS::Authentication::SecurityLevel securityLevel) noexcept
    {
        pImpl->setSecurityLevel(securityLevel);
    } 
    void setMinimumUserPrivileges(const UMPS::Authentication::UserPrivileges privileges) noexcept
    {
        return pImpl->setMinimumUserPrivileges(privileges);
    }
    UMPS::Authentication::UserPrivileges getMinimumUserPrivileges() const noexcept
    {
        return pImpl->getMinimumUserPrivileges();
    }
    UMPS::Authentication::SecurityLevel getSecurityLevel() const noexcept
    {
        return pImpl->getSecurityLevel();
    }
    UMPS::Services::ConnectionInformation::SocketType getSocketType() const noexcept
    {
        return pImpl->getSocketType();
    } 
    void setConnectOrBind(
        const UMPS::Services::ConnectionInformation::ConnectOrBind connectOrBind) noexcept
    {
        pImpl->setConnectOrBind(connectOrBind);
    }
    UMPS::Services::ConnectionInformation::ConnectOrBind getConnectOrBind() const noexcept
    {
        return pImpl->getConnectOrBind();
    }
    void clear() noexcept
    {
        pImpl->clear();
    }
    ~XPublisher() = default;
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::SocketDetails::XPublisher> pImpl;
};


//----------------------------------------------------------------------------//
class Details
{
public:
    Details() :
        pImpl(std::make_unique<UMPS::Services::ConnectionInformation::Details> ())
    {
    }
    Details(const Details &details)
    {
        *this = details;
    }
    Details(Details &&details) noexcept
    {
        *this = std::move(details);
    }
    Details& operator=(const Details &details)
    {
        if (&details == this){return *this;}
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Details> (*details.pImpl);
        return *this;
    }
    Details& operator=(const UMPS::Services::ConnectionInformation::Details &details)
    {
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Details> (details);
        return *this;
    }
    Details& operator=(Details &&details) noexcept
    {
        if (&details == this){return *this;}
        pImpl = std::move(details.pImpl);
        return *this;
    }
    UMPS::Services::ConnectionInformation::Details& getNativeClassReference() const
    {
        return *pImpl;
    }
    void setName(const std::string &name)
    {
        pImpl->setName(name);
    }
    std::string getName() const
    {
        return pImpl->getName();
    }
    ::Subscriber getSubscriberSocketDetails() const
    {
        return ::Subscriber(pImpl->getSubscriberSocketDetails());
    }
    ::Publisher getPublisherSocketDetails() const
    {
        return ::Publisher(pImpl->getPublisherSocketDetails());
    }
    UMPS::Services::ConnectionInformation::SocketType getSocketType() const
    {
        return pImpl->getSocketType();
    }
    ~Details() = default;
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::Details> pImpl;
};

///--------------------------------------Requestor---------------------------///
class Requestor
{
public:
    Requestor() :
        pImpl(std::make_unique<UMPS::Services::ConnectionInformation::Requestor> ())
    {
    }
    explicit Requestor(UMPS::Python::Messaging::Context &context)
    {
        auto umpsContext = context.getSharedPointer();
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Requestor> (umpsContext);
    }
    explicit Requestor(UMPS::Python::Logging::ILog &logger)
    {
        auto umpsLogger = logger.getSharedPointer();
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Requestor> (umpsLogger);
    }
    Requestor(UMPS::Python::Messaging::Context &context,
              UMPS::Python::Logging::ILog &logger)
    {
        auto umpsContext = context.getSharedPointer();
        auto umpsLogger = logger.getSharedPointer();
        pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Requestor> (umpsContext, umpsLogger);
    }
    void initialize(const UMPS::Python::Services::ConnectionInformation::RequestorOptions &options)
    {
        pImpl->initialize(options.getNativeClassReference());
    }
    [[nodiscard]] bool isInitialized() const noexcept
    {
        return pImpl->isInitialized();
    }
    [[nodiscard]] UMPS::Python::Authentication::ZAPOptions getZAPOptions() const
    {
        return UMPS::Python::Authentication::ZAPOptions {pImpl->getZAPOptions()};
    }
    void disconnect()
    {
        pImpl->disconnect();
    }
    ::XSubscriber getProxyBroadcastFrontendDetails(const std::string &name)
    {
        return ::XSubscriber {pImpl->getProxyBroadcastFrontendDetails(name)};
    }
    ::XPublisher  getProxyBroadcastBackendDetails(const std::string &name)
    {
        return ::XPublisher {pImpl->getProxyBroadcastBackendDetails(name)};
    }
/*
    ::Router getProxyServiceFrontendDetails(const std::string &name);
    ::Dealer getProxyServiceBackendDetails(const std::string &name);
*/
/*
    explicit PublisherOptions(const UMPS::Messaging::PublisherSubscriber::PublisherOptions &options);
    PublisherOptions(PublisherOptions &&options) noexcept;
    PublisherOptions& operator=(const PublisherOptions &options);
    PublisherOptions& operator=(const  UMPS::Messaging::PublisherSubscriber::PublisherOptions &options);
    PublisherOptions& operator=(PublisherOptions &&options) noexcept;
*/
//    ~Requestor() = default;
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::Requestor> pImpl;
};

}

void UMPS::Python::Services::initialize(
    pybind11::module &m) 
{
    pybind11::module servicesModule = m.def_submodule("Services");
    servicesModule.attr("__doc__")
        = "Service-oriented messaging patterns used in UMPS.";
    ///----------------------------------------------------------------------///
    ///                       Connection Information                         ///
    ///----------------------------------------------------------------------///
    pybind11::module ciModule = servicesModule.def_submodule("ConnectionInformation");
    ciModule.attr("__doc__") = "The connection information module allows user to interact with the uOperator.";
    // Enums
    pybind11::class_<UMPS::Python::Services::ConnectionInformation::AvailableConnectionsResponse> 
        ciAvailableConnectionsResponse(ciModule, "AvailableConnectionsResponse");
    ciAvailableConnectionsResponse.def(pybind11::init<> ());
    ciAvailableConnectionsResponse.doc() = R""""(
The connections available through the uOperator service.
)"""";
    pybind11::enum_<UMPS::Services::ConnectionInformation::ConnectionType> (ciModule, "ConnectionType")
        .value("Service", UMPS::Services::ConnectionInformation::ConnectionType::Service,
               "The connection is a service.  The socket will be a generaliation of a request-reply pattern.")
        .value("Broadast", UMPS::Services::ConnectionInformation::ConnectionType::Broadcast,
               "The connection is a broadcast.  The socket will a generalization of a publish-subscriber pattern.");
    pybind11::enum_<UMPS::Services::ConnectionInformation::ConnectOrBind> (ciModule, "ConnectOrBind")
        .value("Connect", UMPS::Services::ConnectionInformation::ConnectOrBind::Connect,
               "To connect to the socket peers connect.")
        .value("Bind", UMPS::Services::ConnectionInformation::ConnectOrBind::Bind,
               "To connect to the socket peers bind.");
    pybind11::enum_<UMPS::Services::ConnectionInformation::SocketType> (ciModule, "SocketType")
        .value("Unknown", UMPS::Services::ConnectionInformation::SocketType::Unknown,
               "The socket type is unknown")
        .value("Request", UMPS::Services::ConnectionInformation::SocketType::Request,
               "This is a socket that receives requests from a Response or Dealer")
        .value("Dealer", UMPS::Services::ConnectionInformation::SocketType::Dealer,
               "The is a socket that receives asynchronous requests.  This will accept connections from a Router, Resonse, or Dealer socket")
        .value("Reply", UMPS::Services::ConnectionInformation::SocketType::Reply,
               "This is a socket that receives requests from a Response or Dealer")
        .value("Response", UMPS::Services::ConnectionInformation::SocketType::Response,
               "This is a socket that receives responses from a Request or Router")
        .value("Router", UMPS::Services::ConnectionInformation::SocketType::Router,
               "This is a socket that receives asynchronous responses from a Dealer, Request, or Router")
        .value("Publisher", UMPS::Services::ConnectionInformation::SocketType::Publisher,
               "This is a socket that allows connections from a Subscriber or XSubscriber")
        .value("XPublisher", UMPS::Services::ConnectionInformation::SocketType::XPublisher,
               "This is an extended publisher socket that accepts connections from a Subscriber or XSubscriber")
        .value("Subscriber", UMPS::Services::ConnectionInformation::SocketType::Subscriber,
               "This is a socket that allows connections from a Publisher or XPublisher")
        .value("XSubscriber", UMPS::Services::ConnectionInformation::SocketType::XSubscriber,
               "This is an extended subscriber that allows connections from a Publisher or XPublisher")
        .value("Proxy", UMPS::Services::ConnectionInformation::SocketType::Proxy,
               "This is not a socket but a paradigm.  It indicates that there could be a Request-Router or XPublisher-XSubsccriber.  For this socket type the user will have to get the frontend and backend connections details.");
    pybind11::enum_<UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode> (ciAvailableConnectionsResponse, "ReturnCode")
        .value("Success", UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode::Success,
               "The request was successful.")
        .value("NoItem", UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode::NoItem,
               "The requested item was not present.")
        .value("InvalidMessage", UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode::InvalidMessage,
               "The request message was invalid.")
        .value("AlgorithmFailure", UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode::AlgorithmFailure,
               "A server-side error was encountered.");
    ///----------------------------Sockets-----------------------------------///
    pybind11::module socketModule = servicesModule.def_submodule("SocketDetails");
    socketModule.attr("__doc__") = "This defines the different types of sockets in UMPS.";
    pybind11::class_<::Subscriber> subscriber(socketModule, "Subscriber");
    subscriber.def(pybind11::init<> ());
    subscriber.doc() = R""""(
This defines a subscriber socket.

Properties
    address : str
        The address of the class. 
    security_level : enum
        The socket's security level.
    connect_or_bind : enum
        Determines if peers should connect or bind.
    minimum_user_privileges : enum
        The minimum privileges required to connect (bind) to this socket.

Read-Only Properties
    socket_type : The socket type.
)""""; 
    subscriber.def("__copy__", [](const ::Subscriber &self)
    {
        return ::Subscriber(self);
    }); 
    subscriber.def_property("address",
                            &::Subscriber::getAddress,
                            &::Subscriber::setAddress);
    subscriber.def_property("security_level",
                            &::Subscriber::getSecurityLevel,
                            &::Subscriber::setSecurityLevel);
    subscriber.def_property("connect_or_bind",
                            &::Subscriber::getConnectOrBind,
                            &::Subscriber::setConnectOrBind);
    subscriber.def_property("minimum_user_privileges",
                            &::Subscriber::getMinimumUserPrivileges,
                            &::Subscriber::setMinimumUserPrivileges);
    subscriber.def_property_readonly("socket_type",
                                     &::Subscriber::getSocketType);

    pybind11::class_<::Publisher> publisher(socketModule, "Publisher");
    publisher.def(pybind11::init<> ());
    publisher.doc() = R""""(
This defines a publisher socket.

Properties
    address : str
        The address of the class. 
    security_level : enum
        The socket's security level.
    connect_or_bind : enum
        Determines if peers should connect or bind.
    minimum_user_privileges : enum
        The minimum privileges required to connect (bind) to this socket.

Read-Only Properties
    socket_type : The socket type.
)""""; 
    publisher.def("__copy__", [](const ::Publisher &self)
    {   
        return ::Publisher(self);
    }); 
    publisher.def_property("address",
                           &::Publisher::getAddress,
                           &::Publisher::setAddress);
    publisher.def_property("security_level",
                           &::Publisher::getSecurityLevel,
                           &::Publisher::setSecurityLevel);
    publisher.def_property("connect_or_bind",
                           &::Publisher::getConnectOrBind,
                           &::Publisher::setConnectOrBind);
    publisher.def_property("minimum_user_privileges",
                           &::Publisher::getMinimumUserPrivileges,
                           &::Publisher::setMinimumUserPrivileges);
    publisher.def_property_readonly("socket_type",
                                    &::Publisher::getSocketType);

    pybind11::class_<::XSubscriber> xsubscriber(socketModule, "XSubscriber");
    xsubscriber.def(pybind11::init<> ());
    xsubscriber.doc() = R""""(
This defines an extended subscriber socket.

Properties
    address : str
        The address of the class. 
    security_level : enum
        The socket's security level.
    connect_or_bind : enum
        Determines if peers should connect or bind.
    minimum_user_privileges : enum
        The minimum privileges required to connect (bind) to this socket.

Read-Only Properties
    socket_type : The socket type.
)""""; 
    xsubscriber.def("__copy__", [](const ::XSubscriber &self)
    {   
        return ::XSubscriber(self);
    }); 
    xsubscriber.def_property("address",
                             &::XSubscriber::getAddress,
                             &::XSubscriber::setAddress);
    xsubscriber.def_property("security_level",
                             &::XSubscriber::getSecurityLevel,
                             &::XSubscriber::setSecurityLevel);
    xsubscriber.def_property("connect_or_bind",
                             &::XSubscriber::getConnectOrBind,
                             &::XSubscriber::setConnectOrBind);
    xsubscriber.def_property("minimum_user_privileges",
                             &::XSubscriber::getMinimumUserPrivileges,
                             &::XSubscriber::setMinimumUserPrivileges);
    xsubscriber.def_property_readonly("socket_type",
                                      &::XSubscriber::getSocketType);

    pybind11::class_<::XPublisher> xpublisher(socketModule, "XPublisher");
    xpublisher.def(pybind11::init<> ());
    xpublisher.doc() = R""""(
This defines an extened publisher socket.

Properties
    address : str
        The address of the class. 
    security_level : enum
        The socket's security level.
    connect_or_bind : enum
        Determines if peers should connect or bind.
    minimum_user_privileges : enum
        The minimum privileges required to connect (bind) to this socket.

Read-Only Properties
    socket_type : The socket type.
)"""";
    xpublisher.def("__copy__", [](const ::XPublisher &self)
    {
        return ::XPublisher(self);
    });
    xpublisher.def_property("address",
                            &::XPublisher::getAddress,
                            &::XPublisher::setAddress);
    xpublisher.def_property("security_level",
                            &::XPublisher::getSecurityLevel,
                            &::XPublisher::setSecurityLevel);
    xpublisher.def_property("connect_or_bind",
                            &::XPublisher::getConnectOrBind,
                            &::XPublisher::setConnectOrBind);
    xpublisher.def_property("minimum_user_privileges",
                            &::XPublisher::getMinimumUserPrivileges,
                            &::XPublisher::setMinimumUserPrivileges);
    xpublisher.def_property_readonly("socket_type",
                                     &::XPublisher::getSocketType);
    ///------------------------------Details---------------------------------///
    pybind11::class_<::Details> ciDetails(ciModule, "Details");
    ciDetails.def(pybind11::init<> ());
    ciDetails.doc() = R""""(
This is a container for holding the details of the socket.

Properties
    name : str
        The name of the socket.

Read-Only Properties
    subscriber_socket_details : Subscriber
        If the socket_type is Subscriber then these are the subscriber socket details.
    socket_type : enum
        The socket type
)"""";
    ciDetails.def("__copy__", [](const ::Details &self)
    {
        return ::Details(self);
    });
    ciDetails.def_property("name",
                           &::Details::getName,
                           &::Details::setName);
    ciDetails.def_property_readonly("socket_type",
                                    &::Details::getSocketType);
    ciDetails.def_property_readonly("publisher_socket_details",
                                    &::Details::getPublisherSocketDetails);
    ciDetails.def_property_readonly("subscriber_socket_details",
                                    &::Details::getSubscriberSocketDetails);
    ///--------------------------Requestor Options---------------------------///
    pybind11::class_<UMPS::Python::Services::ConnectionInformation::RequestorOptions>
        ciRequestorOptions(ciModule, "RequestorOptions");
    ciRequestorOptions.def(pybind11::init<> ());
    ciRequestorOptions.doc() = R""""(
The options to create a uOperator requestor.

Required Properties :
    address : str
        The uOperator's address to which to connect; e.g., tcp://127.0.0.1:5555

Optional Properties :
    zap_options : UMPS::Python::Authentication::ZAPOptions
        The ZeroMQ authenticaiton protocol options for authentication this connection.
    time_out : int
        The number of milliseconds to wait when receiving a response message.
        This is typically useful when the thread has other things to do.
        A negative number number will cause this to block indefinitely.
)"""";
    ciRequestorOptions.def("__copy__", [](const ConnectionInformation::RequestorOptions &self)
    {
        return ConnectionInformation::RequestorOptions(self);
    });
    ciRequestorOptions.def_property("address",
                                    &ConnectionInformation::RequestorOptions::getAddress,
                                    &ConnectionInformation::RequestorOptions::setAddress);
    ciRequestorOptions.def_property("zap_options",
                                    &ConnectionInformation::RequestorOptions::getZAPOptions,
                                    &ConnectionInformation::RequestorOptions::setZAPOptions);
    ciRequestorOptions.def_property("time_out",
                                    &ConnectionInformation::RequestorOptions::getTimeOut,
                                    &ConnectionInformation::RequestorOptions::setTimeOut);
    ///-------------------------------Requestor Client-----------------------/// 
    pybind11::class_<::Requestor> ciRequestor(ciModule, "Requestor");
    ciRequestor.def(pybind11::init<> ());
    ciRequestor.def(pybind11::init<UMPS::Python::Logging::ILog &> ());
    ciRequestor.def(pybind11::init<UMPS::Python::Messaging::Context &> ());
    ciRequestor.def(pybind11::init<UMPS::Python::Messaging::Context &,
                                   UMPS::Python::Logging::ILog &> ());
    ciRequestor.doc() = R""""(
Connects to the uOperator.

Read-Only Properties:
    initialized : bool
        True indicates the requestor is initialized and ready to query the uOperator.
    zap_options : ZAPOptions
        The ZAP options used to connect to the uOperator.  This is useful for
        re-use purposes as more connections are made.
    all_connection_details : List
        A list of all the available connections provided by the uOperator.
)"""";
    ciRequestor.def("initialize",
                    &::Requestor::initialize,
                    "Connects to the uOperator.");
    ciRequestor.def_property_readonly("initialized",
                                      &::Requestor::isInitialized);
    ciRequestor.def_property_readonly("zap_options",
                                      &::Requestor::getZAPOptions);
    ciRequestor.def("get_proxy_broadcast_frontend_details",
                    &::Requestor::getProxyBroadcastFrontendDetails,
                    "Gets the details for connecting to the named frontend of a proxy broadcast.  In ZeroMQ frontends are where data go in so a message producer would use this method.");
    ciRequestor.def("get_proxy_broadcast_backend_details",
                    &::Requestor::getProxyBroadcastBackendDetails,
                    "Gets the details for connecting to the named backend of a proxy broadcast.  In ZeroMQ backends are where data come out so a message consumer would use this method.");
    ciRequestor.def("disconnect",
                    &::Requestor::disconnect,
                    "Disconnects from the uOperator.");
}
