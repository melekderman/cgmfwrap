#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

#include "cgmfEvents.h"
#include "rngcgm.h"

struct EventOutput {
    std::vector<double> neutron_energies;
    std::vector<double> gamma_energies;
    std::vector<double> neutron_dir_cosu;
    std::vector<double> neutron_dir_cosv;
    std::vector<double> neutron_dir_cosw;
    int nu_n;
    int nu_g;
};

namespace {
const char* resolve_data_path() {
    const char* env_path = std::getenv("CGMFDATA");
    if (env_path != nullptr && env_path[0] != '\0') {
        return env_path;
    }
#ifdef CGMF_DATA_PATH_DEFAULT
    return CGMF_DATA_PATH_DEFAULT;
#else
    throw std::runtime_error(
        "CGMF data path is not configured. Set CGMFDATA, or build with "
        "-DCGMF_DATA_DIR=/path/to/CGMF/data."
    );
#endif
}

void initialize_cgmf() {
    static bool initialized = false;
    if (initialized) {
        return;
    }

    setdatapath(resolve_data_path());

    static UniformRNG rng(1);
    set_rng(rng);

    // Intentionally leak the initializer event: CGMF's destructor calls
    // cgmDeleteAllocated, which double-frees its internal allocations during
    // process exit on macOS. Letting the OS reclaim the memory avoids that.
    new cgmfEvent();
    initialized = true;
}
}  // namespace

// ZAID = 1000*Z+A of fissioning nucleus
EventOutput run_single_event(int ZAIDt, double Ein) {

    initialize_cgmf();

    double time = 0.0;
    double timeWindow = 0.0;
    auto event = std::make_unique<cgmfEvent>(ZAIDt, Ein, time, timeWindow);

    EventOutput out;

    // Neutrons
    int nuTot = event->getNeutronNu();

    for (int j = 0; j < nuTot; j++) {
        out.neutron_energies.push_back(event->getNeutronEnergy(j));
        out.neutron_dir_cosu.push_back(event->getNeutronDircosu(j));
        out.neutron_dir_cosv.push_back(event->getNeutronDircosv(j));
        out.neutron_dir_cosw.push_back(event->getNeutronDircosw(j));
    }

    out.nu_n = nuTot;

    // Gammas
    int nugLF = event->getLightFragmentPhotonNu();
    int nugHF = event->getHeavyFragmentPhotonNu();
    int nugTot = nugLF + nugHF;

    for (int j = 0; j < nugTot; j++) {
        out.gamma_energies.push_back(event->getPhotonEnergy(j));
    }

    out.nu_g = nugTot;

    return out;
}

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(_native, m) {
    py::class_<EventOutput>(m, "EventOutput")
        .def(py::init<>())
        .def_readwrite("neutron_energies", &EventOutput::neutron_energies)
        .def_readwrite("gamma_energies", &EventOutput::gamma_energies)
        .def_readwrite("nu_n", &EventOutput::nu_n)
        .def_readwrite("nu_g", &EventOutput::nu_g)
        .def_readwrite("neutron_dir_cosu", &EventOutput::neutron_dir_cosu)
        .def_readwrite("neutron_dir_cosv", &EventOutput::neutron_dir_cosv)
        .def_readwrite("neutron_dir_cosw", &EventOutput::neutron_dir_cosw);

    m.doc() = "CGMF single-event wrapper";

    m.def("run_event", &run_single_event,
          py::arg("ZAIDt"), py::arg("Ein"),
          "Generate a single CGMF fission event");
}
