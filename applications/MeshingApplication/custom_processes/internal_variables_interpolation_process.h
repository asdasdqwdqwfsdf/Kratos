// KRATOS  __  __ _____ ____  _   _ ___ _   _  ____
//        |  \/  | ____/ ___|| | | |_ _| \ | |/ ___|
//        | |\/| |  _| \___ \| |_| || ||  \| | |  _
//        | |  | | |___ ___) |  _  || || |\  | |_| |
//        |_|  |_|_____|____/|_| |_|___|_| \_|\____| APPLICATION
//
//  License:		 BSD License
//                       license: MeshingApplication/license.txt
//
//  Main authors:    Vicente Mataix Ferrándiz
//

#if !defined(KRATOS_INTERNAL_VARIABLES_INTERPOLATION_PROCESS )
#define  KRATOS_INTERNAL_VARIABLES_INTERPOLATION_PROCESS

// System includes

// External includes

// Project includes
#include "utilities/openmp_utils.h"
#include "meshing_application.h"
#include "processes/process.h"
#include "includes/model_part.h"
#include "includes/kratos_parameters.h"
#include "includes/kratos_components.h"
#include "custom_includes/gauss_point_item.h"
// Include the point locator
#include "utilities/binbased_fast_point_locator.h"
// Include the trees
// #include "spatial_containers/bounding_volume_tree.h" // k-DOP
#include "spatial_containers/spatial_containers.h" // kd-tree

namespace Kratos
{
///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

    // Type definitions for the tree
    typedef GaussPointItem                                        PointType;
    typedef PointType::Pointer                             PointTypePointer;
    typedef std::vector<PointTypePointer>                       PointVector;
    typedef PointVector::iterator                             PointIterator;
    typedef std::vector<double>                              DistanceVector;
    typedef DistanceVector::iterator                       DistanceIterator;

    // KDtree definitions
    typedef Bucket< 3ul, PointType, PointVector, PointTypePointer, PointIterator, DistanceIterator > BucketType;
    typedef Tree< KDTreePartition<BucketType> > KDTree;
    
///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/** 
 * @class InternalVariablesInterpolationProcess
 * 
 * @ingroup MeshingApplication
 * 
 * @brief This utilitiy has as objective to interpolate the values inside elements (and conditions?) in a model part, using as input the original model part and the new one
 * @details The process employs the projection.h from MeshingApplication, which works internally using a kd-tree
 * 
 * @author Vicente Mataix Ferrandiz
 */

class InternalVariablesInterpolationProcess 
    : public Process
{
public:
    ///@name Type Definitions
    ///@{

    // General type definitions
    typedef ModelPart::NodesContainerType                    NodesArrayType;
    typedef ModelPart::ElementsContainerType              ElementsArrayType;
    typedef ModelPart::ConditionsContainerType          ConditionsArrayType;
    typedef Node<3>                                                NodeType;
    typedef Geometry<NodeType>                                 GeometryType;

    /// Pointer definition of InternalVariablesInterpolationProcess
    KRATOS_CLASS_POINTER_DEFINITION( InternalVariablesInterpolationProcess );

    ///@}
    ///@name  Enum's
    ///@{

    /**
     * @brief This enum it used to list the different types of interpolations available
     */
    enum class InterpolationTypes {
        CLOSEST_POINT_TRANSFER = 0, /// Closest Point Transfer. It transfer the values from the closest GP
        LEAST_SQUARE_TRANSFER = 1, /// Least-Square projection Transfer. It transfers from the closest GP from the old mesh
        SHAPE_FUNCTION_TRANSFER = 2  /// Shape Function Transfer. It transfer GP values to the nodes in the old mesh and then interpolate to the new mesh using the shape functions all the time
        };

    ///@}
    ///@name Life Cycle
    ///@{

    // Class Constructor

    /**
     * @brief The constructor of the search utility uses the following inputs:
     * @param rOriginMainModelPart The model part from where interpolate values
     * @param rDestinationMainModelPart The model part where we want to interpolate the values
     * @param ThisParameters The parameters containing all the information needed
     */

    InternalVariablesInterpolationProcess(
        ModelPart& rOriginMainModelPart,
        ModelPart& rDestinationMainModelPart,
        Parameters ThisParameters =  Parameters(R"({})")
        );

    ~InternalVariablesInterpolationProcess() override= default;

    ///@}
    ///@name Operators
    ///@{

    void operator()()
    {
        Execute();
    }

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief We execute the search relative to the old and new model part
     * @details There are mainly two ways to interpolate the internal variables (there are three, but just two are behave correctly)
     * - CLOSEST_POINT_TRANSFER: Closest Point Transfer. It transfer the values from the closest GP
     * - LEAST_SQUARE_TRANSFER: Least-Square projection Transfer. It transfers from the closest GP from the old mesh
     * - SHAPE_FUNCTION_TRANSFER: Shape Function Transfer. It transfer GP values to the nodes in the old mesh and then interpolate to the new mesh using the shape functions all the time
     * @note SFT THIS DOESN'T WORK, AND REQUIRES EXTRA STORE
     */
    void Execute() override;

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /************************************ GET INFO *************************************/
    /***********************************************************************************/

    std::string Info() const override
    {
        return "InternalVariablesInterpolationProcess";
    }

    /************************************ PRINT INFO ***********************************/
    /***********************************************************************************/

    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:

    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    // The model parts
    ModelPart& mrOriginMainModelPart;                    /// The origin model part
    ModelPart& mrDestinationMainModelPart;               /// The destination model part
    const unsigned int mDimension;                       /// Dimension size of the space

    // The allocation parameters
    unsigned int mAllocationSize;                        /// Allocation size for the vectors and max number of potential results
    unsigned int mBucketSize;                            /// Bucket size for kd-tree

    // The seatch variables
    double mSearchFactor;                                /// The search factor to be considered
    PointVector mPointListOrigin;                        /// A list that contents the all the gauss points from the origin modelpart

    // Variables to interpolate
    std::vector<Variable<double>> mInternalVariableList; /// The list of variables to interpolate
    InterpolationTypes mThisInterpolationType;           /// The interpolation type considered

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    /**
     * @brief This function creates a lists of gauss points ready for the search
     * @param ThisModelPart The model part to consider
     */

    PointVector CreateGaussPointList(ModelPart& ThisModelPart);

    /**
     * @brief This method interpolate the values of the GP using the Closest Point Transfer method
     */

    void InterpolateGaussPointsClosestPointTransfer();

    /**
     * @brief This method interpolate the values of the GP using the Least-Square projection Transfer method
     */

    void InterpolateGaussPointsLeastSquareTransfer();

    /**
     * @brief This method interpolate the values of the GP using the Shape Function Transfer method
     */

    void InterpolateGaussPointsShapeFunctionTransfer();

    /**
     * @brief This converts the interpolation string to an enum
     * @param Str The string that you want to comvert in the equivalent enum
     * @return Interpolation: The equivalent enum (this requires less memmory than a std::string)
     */

    InterpolationTypes ConvertInter(const std::string& Str);

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class InternalVariablesInterpolationProcess

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

/****************************** INPUT STREAM FUNCTION ******************************/
/***********************************************************************************/

template<class TPointType, class TPointerType>
inline std::istream& operator >> (std::istream& rIStream,
                                  InternalVariablesInterpolationProcess& rThis);

/***************************** OUTPUT STREAM FUNCTION ******************************/
/***********************************************************************************/

template<class TPointType, class TPointerType>
inline std::ostream& operator << (std::ostream& rOStream,
                                  const InternalVariablesInterpolationProcess& rThis)
{
    return rOStream;
}

///@}

}  // namespace Kratos.

#endif // KRATOS_INTERNAL_VARIABLES_INTERPOLATION_PROCESS  defined
