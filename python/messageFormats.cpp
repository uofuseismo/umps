#include <umps/messageFormats/message.hpp>
#include "python/messageFormats.hpp"

using namespace UMPS::Python::MessageFormats;

///--------------------------------------------------------------------------///
///                                    IMessage                              ///
///--------------------------------------------------------------------------///
IMessage::~IMessage() = default;

void IMessage::fromBaseClass(UMPS::MessageFormats::IMessage &)
{
}

std::unique_ptr<IMessage> IMessage::clone(
    const std::unique_ptr<UMPS::MessageFormats::IMessage> &) const
{
    return nullptr;
}

std::unique_ptr<IMessage> IMessage::createInstance() const
{
    return nullptr;
}

std::unique_ptr<UMPS::MessageFormats::IMessage>
    IMessage::getBaseClass() const noexcept
{
    return nullptr;
}
///--------------------------------------------------------------------------///
///                                  Iniitalize                              ///
///--------------------------------------------------------------------------///
void UMPS::Python::MessageFormats::initialize(pybind11::module &m) 
{
    pybind11::module mm = m.def_submodule("MessageFormats");
    mm.attr("__doc__") = "Core message formats used in UMPS.";

    pybind11::class_<UMPS::Python::MessageFormats::IMessage> iMessage(mm, "IMessage");
    iMessage.def(pybind11::init<> ());
    iMessage.doc() = R""""(
This is a Python wrapper to the IMessage abstract message base class in the
C++ library.  This really should not be used in your Python applications.
For this reason, no functionality has been exposed.
)"""";

}
