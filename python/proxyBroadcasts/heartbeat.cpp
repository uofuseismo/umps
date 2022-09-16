#include <sstream>
#include <pybind11/pybind11.h>
#include <umps/proxyBroadcasts/heartbeat/status.hpp>
#include "messageFormats/message.hpp"
#include "proxyBroadcasts.hpp"

namespace UHeartbeat = UMPS::ProxyBroadcasts::Heartbeat;

namespace PUMPS::ProxyBroadcasts
{
class Status : public PUMPS::MessageFormats::IMessage
{
public:
    /// C'tor
    Status() :
        mStatus(std::make_unique<UHeartbeat::Status> ())
    {
    }
    /// Copy c'tor
    Status(const Status &status)
    {
        *this = status;
    }
    /// Move c'tor
    Status(Status &&status) noexcept
    {
        *this = std::move(status);
    }
    /// Destructor
    ~Status() = default;
    /// Copy assignment
    Status& operator=(const Status &status)
    {
        if (&status == this){return *this;}
        mStatus = std::make_unique<UHeartbeat::Status> (*status.mStatus);
        return *this;
    }
    /// Move assignment
    Status& operator=(Status &&status) noexcept
    {
        if (&status == this){return *this;}
        mStatus = std::move(status.mStatus);
        return *this;
    }
    /// Module name
    void setModule(const std::string &module)
    {
        mStatus->setModule(module);
    }
    std::string getModule() const noexcept
    {
        return mStatus->getModule();
    }
    /// Host name
    void setHostName(const std::string &host)
    {
        mStatus->setHostName(host);
    }
    std::string getHostName() const noexcept
    {
        return mStatus->getHostName();
    }
    /// Time stamp
    void setTimeStamp(const std::string &timeStamp)
    {
        mStatus->setTimeStamp(timeStamp);
    }
    std::string getTimeStamp() const noexcept
    {
        return mStatus->getTimeStamp();
    }
    /// Status
    void setStatus(const UHeartbeat::ModuleStatus status) noexcept
    {
        mStatus->setModuleStatus(status);
    }
    UHeartbeat::ModuleStatus getStatus() const noexcept
    {
        return mStatus->getModuleStatus();
    }
    /// Message type
    std::string getMessageType() const noexcept
    {
        return mStatus->getMessageType();
    }
    std::unique_ptr<UHeartbeat::Status> mStatus;
};
}

///--------------------------------------------------------------------------///
///                     Code To Initialize Python Bindings                   ///
///--------------------------------------------------------------------------///
/// Initialize the class
void PUMPS::ProxyBroadcasts::initializeHeartbeat(pybind11::module &m) 
{
    pybind11::module hbm = m.def_submodule("Heartbeat");
    // Enums
    pybind11::enum_<UMPS::ProxyBroadcasts::Heartbeat::ModuleStatus> (hbm, "ModuleStatus")
        .value("Unknown", UMPS::ProxyBroadcasts::Heartbeat::ModuleStatus::Unknown,
               "The module's status is unknown.")
        .value("Alive", UMPS::ProxyBroadcasts::Heartbeat::ModuleStatus::Alive,
               "The module is alive.")
        .value("Disconnected",  UMPS::ProxyBroadcasts::Heartbeat::ModuleStatus::Disconnected,
               "The module is disconnected.")
        .value("Dead",  UMPS::ProxyBroadcasts::Heartbeat::ModuleStatus::Died,
               "The module has died.");

    pybind11::class_<PUMPS::ProxyBroadcasts::Status>
        s(hbm, "Status");
    s.def(pybind11::init<> ());
    s.doc() = R""""(
This is an UMPS module status message.

Required Properties :

   module        : The module's name.
   module_status : The module's status. 

Optional Properties :
   host_name     : The host name.
   time_stamp    : The UTC time stamp in YEAR:MO:DYTHH:MM:SS.XXX format.

Read-Only Properties :
   message_type  : The message type.

)"""";
    s.def("__copy__", [](const Status &self)
    {
        return Status(self);
    });
    s.def("__repr__", [](const Status &self)
    {
        std::stringstream stream;
        stream << *self.mStatus;
        return stream.str();
    });
    s.def_property("module",
                   &Status::getModule,
                   &Status::setModule);
    s.def_property("module_status",
                   &Status::getStatus,
                   &Status::setStatus);
    s.def_property("host_name",
                   &Status::getHostName,
                   &Status::setHostName);
    s.def_property("time_stamp",
                   &Status::getTimeStamp,
                   &Status::setTimeStamp);
    s.def_property_readonly("message_type",
                            &Status::getMessageType);
    // Pickling rules
    s.def(pybind11::pickle(
        [](const Status &status)
        {
           auto module = status.getModule();
           auto hostName = status.getHostName();
           auto timeStamp = status.getTimeStamp();
           auto moduleStatus = static_cast<int> (status.getStatus());
           return pybind11::make_tuple(module, hostName,
                                       timeStamp, moduleStatus);
        },
        [](pybind11::tuple t)
        {
           if (t.size() != 4){throw std::runtime_error("Invalid state");}
           auto module = t[0].cast<std::string> (); 
           auto hostName = t[1].cast<std::string> (); 
           auto timeStamp = t[2].cast<std::string> (); 
           auto moduleStatus
               = static_cast<UHeartbeat::ModuleStatus> (t[3].cast<int> ());
           Status status;
           status.setModule(module);
           status.setHostName(hostName);
           status.setTimeStamp(timeStamp);
           status.setStatus(moduleStatus);
           return status;
        }
    ));

}
