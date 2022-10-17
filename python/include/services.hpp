#ifndef PUMPS_SERVICES_SERVICES_HPP
#define PUMPS_SERVICES_SERVICES_HPP
#include <pybind11/pybind11.h>
namespace PUMPS::Services
{
void initializeCommand(pybind11::module &m);
}
#endif
