//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Authors:        Aditya Ghantasala, https://github.com/adityaghantasala
// 					Navaneeth K Narayanan
//					Rishith Ellath Meethal
//


#include "chimera_application_variables.h"

namespace Kratos
{

KRATOS_CREATE_VARIABLE(double, CHIMERA_DISTANCE);
KRATOS_CREATE_VARIABLE(double, ROTATIONAL_ANGLE);
KRATOS_CREATE_VARIABLE(double, ROTATIONAL_VELOCITY);
KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(ROTATION_MESH_DISPLACEMENT);
KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(ROTATION_MESH_VELOCITY);

// Flag for distinguishing b/w velocity and pressure constraints. Used in fractional-step approach
KRATOS_CREATE_LOCAL_FLAG(ChimeraFlags, CHIMERA_INTERNAL_BOUNDARY, 12);

}
