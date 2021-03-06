//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

#if !defined(KRATOS_STATISTICS_APPLICATION_VARIABLES_H_INCLUDED)
#define KRATOS_STATISTICS_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "containers/variable.h"
#include "includes/define.h"
#include "containers/variable_component.h"
#include "containers/vector_component_adaptor.h"

namespace Kratos
{
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STATISTICS_APPLICATION, VECTOR_3D_SUM)
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STATISTICS_APPLICATION, VECTOR_3D_MEAN)
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STATISTICS_APPLICATION, VECTOR_3D_VARIANCE)

KRATOS_DEFINE_APPLICATION_VARIABLE(STATISTICS_APPLICATION, double, VECTOR_3D_NORM)
KRATOS_DEFINE_APPLICATION_VARIABLE(STATISTICS_APPLICATION, double, SCALAR_NORM)
KRATOS_DEFINE_APPLICATION_VARIABLE(STATISTICS_APPLICATION, double, SCALAR_SUM)
KRATOS_DEFINE_APPLICATION_VARIABLE(STATISTICS_APPLICATION, double, SCALAR_MEAN)
KRATOS_DEFINE_APPLICATION_VARIABLE(STATISTICS_APPLICATION, double, SCALAR_VARIANCE)

} // namespace Kratos

#endif /* KRATOS_STATISTICS_APPLICATION_VARIABLES_H_INCLUDED */
