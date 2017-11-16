//
//   Project Name:        KratosSolidMechanicsApplication $
//   Created by:          $Author:            JMCarbonell $
//   Last modified by:    $Co-Author:                     $
//   Date:                $Date:                July 2013 $
//   Revision:            $Revision:                  0.0 $
//
//

// System includes
#if defined(KRATOS_PYTHON)

// External includes
#include <boost/python.hpp>

// Project includes
#include "custom_python/add_custom_strategies_to_python.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_python/add_custom_constitutive_laws_to_python.h"
#include "custom_python/add_cross_sections_to_python.h"
#include "custom_python/add_custom_processes_to_python.h"

#include "solid_mechanics_application.h"

namespace Kratos
{

namespace Python
{

using namespace boost::python;



BOOST_PYTHON_MODULE(KratosSolidMechanicsApplication)
{

    class_<KratosSolidMechanicsApplication,
           KratosSolidMechanicsApplication::Pointer,
           bases<KratosApplication>, boost::noncopyable >("KratosSolidMechanicsApplication")
           ;

    AddCustomUtilitiesToPython();
    AddCustomStrategiesToPython();
    AddCustomConstitutiveLawsToPython(); 
    AddCrossSectionsToPython();
    AddCustomProcessesToPython();

    //registering variables in python ( if must to be seen from python )

    // Generalized eigenvalue problem
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( BUILD_LEVEL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( EIGENVALUE_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( EIGENVECTOR_MATRIX )

    //For process information
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( LOAD_RESTART )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( COMPUTE_DYNAMIC_TANGENT )
      
   
    //For explicit schemes
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( MIDDLE_VELOCITY )
            
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( POINT_LOAD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( LINE_LOAD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( SURFACE_LOAD )

    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( POINT_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( LINE_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( SURFACE_MOMENT )

    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( POINT_STIFFNESS )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( LINE_STIFFNESS )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( SURFACE_STIFFNESS )

     
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( POINT_LOAD_VECTOR )  
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( LINE_LOAD_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SURFACE_LOAD_VECTOR )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( POSITIVE_FACE_PRESSURE_VECTOR )  
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( NEGATIVE_FACE_PRESSURE_VECTOR )
      
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( POINT_MOMENT_VECTOR )  
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( LINE_MOMENT_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SURFACE_MOMENT_VECTOR )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( POINT_STIFFNESS_VECTOR )  
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( LINE_STIFFNESS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SURFACE_STIFFNESS_VECTOR )

      
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( WRITE_ID )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( RAYLEIGH_ALPHA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( RAYLEIGH_BETA )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( EXTERNAL_FORCES_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( INTERNAL_FORCES_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( CONTACT_FORCES_VECTOR )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( STABILIZATION_FACTOR )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( PRESSURE_REACTION )
      
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( VON_MISES_STRESS )

    //For beams
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( CROSS_SECTION_AREA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( CROSS_SECTION_RADIUS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( CROSS_SECTION_SIDES )
      
    //For shells cross section
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CROSS_SECTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CROSS_SECTION_OUTPUT_PLY_ID )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CROSS_SECTION_OUTPUT_PLY_LOCATION )
    
    //For shell generalized variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRAIN )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRAIN_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CURVATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CURVATURE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_FORCE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_FORCE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_MOMENT_GLOBAL )

    //reading beam section properties
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SECTION_HEIGHT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SECTION_WIDTH  )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( INERTIA_X )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( INERTIA_Y )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SECTION_SIZE )
    
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( YOUNGxAREA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( YOUNGxINERTIA_X )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( YOUNGxINERTIA_Y )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHEARxREDUCED_AREA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHEARxPOLAR_INERTIA )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( NEWMARK_BETA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( NEWMARK_GAMMA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( BOSSAK_ALPHA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( EQUILIBRIUM_POINT )
      
}


}  // namespace Python.

}  // namespace Kratos.

#endif // KRATOS_PYTHON defined
