#ifndef PUMPS_MESSAGING_HPP
#define PUMPS_MESSAGING_HPP
#include <pybind11/pybind11.h>
namespace PUMPS::Messaging
{
void initializeMessaging(pybind11::module &m);
}
#endif
