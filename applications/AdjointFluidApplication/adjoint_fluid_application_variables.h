//  KratosAdjointFluidApplication
//
//  License:		 BSD License
//					 license: AdjointFluidApplication/license.txt
//
//  Main authors:    Michael Andre, https://github.com/msandre
//

#if !defined(KRATOS_ADJOINT_FLUID_APPLICATION_VARIABLES_H_INCLUDED)
#define  KRATOS_ADJOINT_FLUID_APPLICATION_VARIABLES_H_INCLUDED

// Project includes
#include "includes/define.h"
#include "includes/kratos_application.h"
#include "includes/variables.h"
#include "includes/dem_variables.h"

namespace Kratos
{

    KRATOS_DEFINE_VARIABLE(double, NUMERICAL_DIFFUSION )
    KRATOS_DEFINE_VARIABLE( Matrix, VMS_STEADY_TERM_PRIMAL_GRADIENT_MATRIX)

    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_1_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_2_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_3_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_4_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_5_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_6_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_7_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_8_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_9_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_10_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_11_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_12_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_13_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_14_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_15_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_16_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_17_EIGEN_MIN)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_18_EIGEN_MIN)

    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_1_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_2_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_3_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_4_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_5_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_6_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_7_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_8_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_9_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_10_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_11_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_12_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_13_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_14_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_15_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_16_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_17_EIGEN_MAX)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_18_EIGEN_MAX)

    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_1_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_2_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_3_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_4_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_5_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_6_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_7_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_8_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_9_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_10_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_11_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_12_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_13_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_14_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_15_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_16_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_17_ENERGY)
    KRATOS_DEFINE_VARIABLE( double, SYMMETRIC_MATRIX_18_ENERGY)

// Define variables
// Moved to Kratos Core for trilinos_application
//KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( ADJOINT_FLUID_VECTOR_1 )
//KRATOS_DEFINE_VARIABLE(double, ADJOINT_FLUID_SCALAR_1 )
//KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( PRIMAL_VELOCITY )
//KRATOS_DEFINE_VARIABLE(double, PRIMAL_PRESSURE )
//KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( SHAPE_SENSITIVITY )
//KRATOS_DEFINE_VARIABLE(double, NORMAL_SENSITIVITY )
}

#endif	/* KRATOS_ADJOINT_FLUID_APPLICATION_VARIABLES_H_INCLUDED */
