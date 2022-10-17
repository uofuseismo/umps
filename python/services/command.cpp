#include <umps/services/command/terminateRequest.hpp>
#include <umps/services/command/terminateResponse.hpp>
#include "services.hpp"
#include "messageFormats/message.hpp"
#include <pybind11/pybind11.h>

namespace UCommand = UMPS::Services::Command;

namespace
{

class TerminateRequest : public PUMPS::MessageFormats::IMessage
{
public:
    TerminateRequest() :
        pImpl(std::make_unique<UCommand::TerminateRequest> ())
    {
    }
    TerminateRequest(const TerminateRequest &request)
    {
        *this = request;
    }
    TerminateRequest(TerminateRequest &&request) noexcept
    {
        *this = std::move(request);
    }
    TerminateRequest& operator=(const TerminateRequest &request)
    {
        if (&request == this){return *this;}
        pImpl = std::make_unique<UCommand::TerminateRequest> (*request.pImpl);
        return *this;
    }
    TerminateRequest& operator=(TerminateRequest &&request) noexcept
    {
        if (&request == this){return *this;}
        pImpl = std::move(request.pImpl);
        return *this;
    }
    ~TerminateRequest() = default;
    std::string getMessageType() const noexcept
    {
        return pImpl->getMessageType();
    }
    std::unique_ptr<UCommand::TerminateRequest> pImpl;
};
//----------------------------------------------------------------------------//
class TerminateResponse : public PUMPS::MessageFormats::IMessage
{
public:
    TerminateResponse() :
        pImpl(std::make_unique<UCommand::TerminateResponse> ()) 
    {
    }
    TerminateResponse(const TerminateResponse &response)
    {   
        *this = response;
    }   
    TerminateResponse(TerminateResponse &&response) noexcept
    {   
        *this = std::move(response);
    }   
    TerminateResponse& operator=(const TerminateResponse &response)
    {   
        if (&response == this){return *this;}
        pImpl = std::make_unique<UCommand::TerminateResponse> (*response.pImpl);
        return *this;
    }   
    TerminateResponse& operator=(TerminateResponse &&response) noexcept
    {   
        if (&response == this){return *this;}
        pImpl = std::move(response.pImpl);
        return *this;
    }
    ~TerminateResponse() = default;
    void setReturnCode(const UCommand::TerminateResponse::ReturnCode rc) noexcept
    {
        pImpl->setReturnCode(rc);
    }
    UCommand::TerminateResponse::ReturnCode getReturnCode() const
    {
        if (!haveReturnCode())
        {
            throw std::runtime_error("Return code not set");
        }
        return pImpl->getReturnCode(); 
    }
    std::string getMessageType() const noexcept
    {
        return pImpl->getMessageType();
    }
    bool haveReturnCode() const noexcept {return pImpl->haveReturnCode();}
    std::unique_ptr<UMPS::Services::Command::TerminateResponse> pImpl;
};

}

void PUMPS::Services::initializeCommand(pybind11::module &m)
{
    ///----------------------------------------------------------------------///
    ///                           Terminate Request                          ///
    ///----------------------------------------------------------------------///
    pybind11::module cm = m.def_submodule("Command");
    // Enums
    pybind11::enum_<UMPS::Services::Command::TerminateResponse::ReturnCode> (cm, "TerminateResponseReturnCode")
        .value("Success",
               UMPS::Services::Command::TerminateResponse::ReturnCode::Success,
               "The module successfully terminated.")
        .value("InvalidCommand",
               UMPS::Services::Command::TerminateResponse::ReturnCode::InvalidCommand,
               "The termination request resulted in an invalid command.  This is a server-side error.")
        .value("ApplicationError",
               UMPS::Services::Command::TerminateResponse::ReturnCode::ApplicationError,
               "The termination request was valid but improperly handled by the server.");
    // Terminate request
    pybind11::class_<::TerminateRequest> terminateRequest(cm, "TerminateRequest");
    terminateRequest.def(pybind11::init<> ());
    terminateRequest.doc() = R""""(
This is an UMPS module termination request message.

Read-Only Properties :
   message_type  : The message type.

)"""";
    terminateRequest.def("__copy__", [](const TerminateRequest &self)
    {
        return ::TerminateRequest(self);
    });
    //------------------------------------------------------------------------//
    // Terminate response
    pybind11::class_<::TerminateResponse> terminateResponse(cm, "TerminateResponse");
    terminateResponse.def(pybind11::init<> ());
    terminateResponse.doc() = R""""(
This is an UMPS module termination response message.

Required Properties :
   return_code : The return code from the module that has been asked to
                 terminate.

Read-Only Properties :
   message_type  : The message type.

)"""";
    // Terminate response
    terminateResponse.def("__copy__", [](const TerminateResponse &self)
    {   
        return ::TerminateResponse(self);
    });
    terminateResponse.def_property("return_code",
                                   &TerminateResponse::getReturnCode,
                                   &TerminateResponse::setReturnCode);
    // Pickling rules
    terminateResponse.def(pybind11::pickle(
        [](const TerminateResponse &response)
        {
           int rc = -1000;
           if (response.haveReturnCode())
           {
               rc = static_cast<int> (response.getReturnCode());
           }
           return pybind11::make_tuple(rc);
        },
        [](pybind11::tuple t)
        {
           if (t.size() != 1){throw std::runtime_error("Invalid state");}
           TerminateResponse response;
           auto irc = t[0].cast<int> (); 
           if (irc != -1000)
           {
               auto rc
                   = static_cast<UCommand::TerminateResponse::ReturnCode> (irc);
               response.setReturnCode(rc);
           }
           return response;
        }
    )); 
/*
    s.def("__repr__", [](const Status &self)
    {
        std::stringstream stream;
        stream << *self.mStatus;
        return stream.str();
    });
*/
}
