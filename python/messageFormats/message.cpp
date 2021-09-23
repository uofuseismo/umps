#include <pybind11/pybind11.h>
#include "messageFormats/message.hpp"
#include "umps/messageFormats/message.hpp"

void PUMPS::MessageFormats::initializeIMessage(pybind11::module &m)
{
    pybind11::class_<PUMPS::MessageFormats::IMessage> o(m, "IMessage");
}
