#include "adjoint_fluid_application.h"

namespace Kratos
{
KratosAdjointFluidApplication::KratosAdjointFluidApplication() :
    KratosApplication("AdjointFluidApplication"),
    mVMSAdjointElement2D(0,Element::GeometryType::Pointer(new Triangle2D3<Node<3> >(Element::GeometryType::PointsArrayType(3)))),
    mVMSAdjointElement3D(0,Element::GeometryType::Pointer(new Tetrahedra3D4<Node<3> >(Element::GeometryType::PointsArrayType(4))))
{}

void KratosAdjointFluidApplication::Register()
{
  // calling base class register to register Kratos components
  KratosApplication::Register();
  std::cout << "Initializing KratosAdjointFluidApplication... " << std::endl;

  // Register elements
  KRATOS_REGISTER_ELEMENT( "VMSAdjointElement2D", mVMSAdjointElement2D );
  KRATOS_REGISTER_ELEMENT( "VMSAdjointElement3D", mVMSAdjointElement3D );

  KRATOS_REGISTER_VARIABLE(NUMERICAL_DIFFUSION);
  KRATOS_REGISTER_VARIABLE(VMS_STEADY_TERM_PRIMAL_GRADIENT_MATRIX);

  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_1_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_2_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_3_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_4_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_5_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_6_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_7_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_8_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_9_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_10_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_11_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_12_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_13_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_14_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_15_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_16_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_17_EIGEN_MIN);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_18_EIGEN_MIN);

  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_1_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_2_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_3_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_4_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_5_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_6_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_7_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_8_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_9_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_10_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_11_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_12_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_13_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_14_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_15_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_16_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_17_EIGEN_MAX);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_18_EIGEN_MAX);

  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_1_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_2_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_3_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_4_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_5_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_6_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_7_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_8_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_9_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_10_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_11_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_12_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_13_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_14_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_15_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_16_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_17_ENERGY);
  KRATOS_REGISTER_VARIABLE(SYMMETRIC_MATRIX_18_ENERGY);
      
  // Register variables
  // Moved to Kratos Core for trilinos_application
  //KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ADJOINT_FLUID_VECTOR_1 );
  //KRATOS_REGISTER_VARIABLE( ADJOINT_FLUID_SCALAR_1 );
  //KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( PRIMAL_VELOCITY );
  //KRATOS_REGISTER_VARIABLE( PRIMAL_PRESSURE );
  //KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( SHAPE_SENSITIVITY );
  //KRATOS_REGISTER_VARIABLE( NORMAL_SENSITIVITY );
}

} // namespace Kratos
