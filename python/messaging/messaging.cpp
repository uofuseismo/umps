#include <umps/messaging/context.hpp>
#include "python/messaging.hpp"

namespace
{

}

using namespace UMPS::Python::Messaging;

///--------------------------------------------------------------------------///
///                              Context                                     ///
///--------------------------------------------------------------------------///

Context::Context(const int nInputThreads) :
    mContext(std::make_shared<UMPS::Messaging::Context> (nInputThreads))
{
}

Context::~Context() = default;

std::shared_ptr<UMPS::Messaging::Context> Context::getSharedPointer() const
{
    return mContext;
}

///--------------------------------------------------------------------------///
///                            Initialization                                ///
///--------------------------------------------------------------------------///
void UMPS::Python::Messaging::initialize(pybind11::module &m)
{
    pybind11::module messagingModule = m.def_submodule("Messaging");
    messagingModule.attr("__doc__") = "Message passing patterns used in UMPS.";
    pybind11::class_<UMPS::Python::Messaging::Context> context(messagingModule, "Context");
    context.attr("__doc__") = "Core messaging utilities for UMPS.";
    context.def(pybind11::init<int> ());
    context.doc() = R""""(
This is an UMPS context.  Loosely speaking, a context is like a thread
that is specific to ZeroMQ and is responsible for sending/receiving
messages.  Typically, you'll want to have one context per each 1 Gb/s
of messaging.  Note, for inproc communication the number of threads can
be 0.
)"""";
     
}
