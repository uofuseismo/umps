#include "messageFormats/pick.hpp"
#include "urts/version.hpp"
#include <pybind11/pybind11.h>

PYBIND11_MODULE(pyurts, m)
{
    m.attr("__version__") = URTS_VERSION;
    m.attr("__name__") = "pyurts";
    m.attr("__doc__") = "A toolkit for real-time seismic monitoring at University of Utah Seismograph Stations.";

    pybind11::module messageFormatsModule = m.def_submodule("MessageFormats");
    messageFormatsModule.attr("__doc__") = "An assortment of message formats in URTS.";
    PURTS::MessageFormats::initializePick(messageFormatsModule);

}


