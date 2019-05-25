//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Author1 Fullname
//                   Author2 Fullname
//

// System includes

#if defined(KRATOS_PYTHON)
// External includes
#include <pybind11/pybind11.h>

// Project includes
#include "custom_python/add_custom_processes_to_python.h"
#include "custom_python/add_custom_strategies_to_python.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "includes/define.h"
#include "rans_modelling_application.h"
#include "rans_modelling_application_variables.h"

namespace Kratos
{
namespace Python
{
PYBIND11_MODULE(KratosRANSModellingApplication, m)
{
    namespace py = pybind11;

    py::class_<KratosRANSModellingApplication, KratosRANSModellingApplication::Pointer, KratosApplication>(
        m, "KratosRANSModellingApplication")
        .def(py::init<>());

    AddCustomStrategiesToPython(m);
    AddCustomUtilitiesToPython(m);
    AddCustomProcessesToPython(m);

    // registering variables in python
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_KINETIC_ENERGY)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_ENERGY_DISSIPATION_RATE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_KINETIC_ENERGY_RATE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_ENERGY_DISSIPATION_RATE_2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IS_CO_SOLVING_PROCESS_ACTIVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OLD_CONVERGENCE_VARIABLE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_Y_PLUS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_AUXILIARY_VARIABLE_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_AUXILIARY_VARIABLE_2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, WALL_SMOOTHNESS_BETA)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, WALL_VON_KARMAN)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENCE_RANS_C_MU)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENCE_RANS_C1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENCE_RANS_C2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_VISCOSITY_MIN)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_VISCOSITY_MAX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_KINETIC_ENERGY_SIGMA)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA)

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_SCALAR_1_ADJOINT_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_SCALAR_2_ADJOINT_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_SCALAR_1_ADJOINT_3)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_SCALAR_2_ADJOINT_3)
}

} // namespace Python.
} // namespace Kratos.

#endif // KRATOS_PYTHON defined
