#ifndef PUMPS_LOGGING_LOG_HPP
#define PUMPS_LOGGING_LOG_HPP
#include <pybind11/pybind11.h>
namespace PUMPS::Logging
{
void initializeLogging(pybind11::module &m);
}
#endif
