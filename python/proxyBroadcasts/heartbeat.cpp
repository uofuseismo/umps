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

    std::unique_ptr<UHeartbeat::Status> mStatus;
};
}

/// Initialize the class
//void PUMPS::ProxyBroadcasts::
void PUMPS::ProxyBroadcasts::initializeHeartbeat(pybind11::module &m) 
{
    pybind11::class_<PUMPS::ProxyBroadcasts::Status>
        o(m, "Status");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This is an UMPS status message.

Read-only Properties :

   is_initialized : True indicates the subscriber is initialized and ready
                    to receive messages.

)"""";
}
