// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

// System includes
#if !defined(KRATOS_ACOUSTIC_STRUCTURE_LINE_CONDITION_H_INCLUDED )
#define  KRATOS_ACOUSTIC_STRUCTURE_LINE_CONDITION_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/condition.h"
#include "includes/variables.h"
//TODO remove this!
// #include "../../StructuralMechanicsApplication/structural_mechanics_application_variables.h"
// #include "mor_application_variables.h"
// #include "custom_conditions/base_load_condition.h"

namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

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
 * @class AcousticStructureLineCondition
 * @ingroup StructuralMechanicsApplication
 * @brief This class is the responsible to add the contributions of the RHS and LHS of the line loads of the structure
 * @details It allows to consider different types of pressure and line loads
 * @tparam TDim The dimension of the condition
 * @author Vicente Mataix Ferrandiz
 */
template<std::size_t TDim>
class AcousticStructureLineCondition
    : public Condition
{
public:
    ///@name Type Definitions
    ///@{

    /// We define the base class BaseLoadCondition
    typedef Condition BaseType;

    /// Dfinition of the index type
    typedef BaseType::IndexType IndexType;

    /// Definition of the size type
    typedef BaseType::SizeType SizeType;

    /// Definition of the node type
    typedef BaseType::NodeType NodeType;

    /// Definition of the properties type
    typedef BaseType::PropertiesType PropertiesType;

    /// Definition of the geometry type with given NodeType
    typedef BaseType::GeometryType GeometryType;

    /// Definition of nodes container type, redefined from GeometryType
    typedef BaseType::NodesArrayType NodesArrayType;

    /// Counted pointer of AcousticStructureLineCondition
    KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION( AcousticStructureLineCondition );

    ///@}
    ///@name Life Cycle
    ///@{

    // Constructor using an array of nodes
    AcousticStructureLineCondition(
        IndexType NewId,
        GeometryType::Pointer pGeometry
        );

    // Constructor using an array of nodes with properties
    AcousticStructureLineCondition(
        IndexType NewId,
        GeometryType::Pointer pGeometry,
        PropertiesType::Pointer pProperties
        );

    /// Destructor.
    ~AcousticStructureLineCondition() override;

    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Creates a new condition pointer
     * @param NewId the ID of the new condition
     * @param ThisNodes the nodes of the new condition
     * @param pProperties the properties assigned to the new condition
     * @return a Pointer to the new condition
     */
    Condition::Pointer Create(
        IndexType NewId,
        NodesArrayType const& ThisNodes,
        PropertiesType::Pointer pProperties
        ) const override;

    /**
     * @brief Creates a new condition pointer
     * @param NewId the ID of the new condition
     * @param pGeom the geometry to be employed
     * @param pProperties the properties assigned to the new condition
     * @return a Pointer to the new condition
     */
    Condition::Pointer Create(
        IndexType NewId,
        GeometryType::Pointer pGeom,
        PropertiesType::Pointer pProperties
        ) const override;

    /**
     * @brief Creates a new condition pointer and clones the previous condition data
     * @param NewId the ID of the new condition
     * @param ThisNodes the nodes of the new condition
     * @return a Pointer to the new condition
     */
    Condition::Pointer Clone (
        IndexType NewId,
        NodesArrayType const& ThisNodes
        ) const override;

    /**
     * @brief Sets on rResult the ID's of the element degrees of freedom
     * @param rResult The vector containing the equation id
     * @param rCurrentProcessInfo The current process info instance
     */
    void EquationIdVector(
        EquationIdVectorType& rResult,
        ProcessInfo& rCurrentProcessInfo
        ) override;

    /**
     * @brief Sets on rElementalDofList the degrees of freedom of the considered element geometry
     * @param rElementalDofList The vector containing the dof of the element
     * @param rCurrentProcessInfo The current process info instance
     */
    void GetDofList(
        DofsVectorType& ElementalDofList,
        ProcessInfo& rCurrentProcessInfo
        ) override;

    /**
     * @brief Sets on rValues the nodal displacements
     * @param rValues The values of displacements
     * @param Step The step to be computed
     */
    void GetValuesVector(
        Vector& rValues,
        int Step = 0
        ) override;

    // /**
    //  * @brief Sets on rValues the nodal velocities
    //  * @param rValues The values of velocities
    //  * @param Step The step to be computed
    //  */
    // void GetFirstDerivativesVector(
    //     Vector& rValues,
    //     int Step = 0
    //     ) override;

    // /**
    //  * @brief Sets on rValues the nodal accelerations
    //  * @param rValues The values of accelerations
    //  * @param Step The step to be computed
    //  */
    // void GetSecondDerivativesVector(
    //     Vector& rValues,
    //     int Step = 0
    //     ) override;

    /**
     * @brief This function provides a more general interface to the element.
     * @details It is designed so that rLHSvariables and rRHSvariables are passed to the element thus telling what is the desired output
     * @param rLeftHandSideMatrices container with the output left hand side matrices
     * @param rLHSVariables paramter describing the expected LHSs
     * @param rRightHandSideVectors container for the desired RHS output
     * @param rRHSVariables parameter describing the expected RHSs
     */
    void CalculateLocalSystem(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo
        ) override;

    /**
      * @brief This is called during the assembling process in order to calculate the elemental right hand side vector only
      * @param rRightHandSideVector the elemental right hand side vector
      * @param rCurrentProcessInfo the current process info instance
      */
    void CalculateRightHandSide(
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo
        ) override;

    /**
      * @brief This is called during the assembling process in order to calculate the elemental mass matrix
      * @param rMassMatrix the elemental mass matrix
      * @param rCurrentProcessInfo The current process info instance
      */
    void CalculateMassMatrix(
        MatrixType& rMassMatrix,
        ProcessInfo& rCurrentProcessInfo
        ) override;

    /**
      * @brief This is called during the assembling process in order to calculate the elemental damping matrix
      * @param rDampingMatrix the elemental damping matrix
      * @param rCurrentProcessInfo The current process info instance
      */
    void CalculateDampingMatrix(
        MatrixType& rDampingMatrix,
        ProcessInfo& rCurrentProcessInfoO
        ) override;

    //  /**
    //   * @brief This function is designed to make the element to assemble an rRHS vector identified by a variable rRHSVariable by assembling it to the nodes on the variable rDestinationVariable.
    //   * @param rRHSVector input variable containing the RHS vector to be assembled
    //   * @param rRHSVariable variable describing the type of the RHS vector to be assembled
    //   * @param rDestinationVariable variable in the database to which the rRHSvector will be assembled
    //   * @param rCurrentProcessInfo The current process info instance
    //  */
    // void AddExplicitContribution(const VectorType& rRHS,
    //     const Variable<VectorType>& rRHSVariable,
    //     Variable<array_1d<double,3> >& rDestinationVariable,
    //     const ProcessInfo& rCurrentProcessInfo
    //     ) override;

    /**
     * @brief This function provides the place to perform checks on the completeness of the input.
     * @details It is designed to be called only once (or anyway, not often) typically at the beginning of the calculations, so to verify that nothing is missing from the input or that no common error is found.
     * @param rCurrentProcessInfo The current process info instance
     */
    int Check( const ProcessInfo& rCurrentProcessInfo ) override;

    /**
     * @brief Check if Rotational Dof existant
     * @return Trues if exists, false otherwise
     */
    virtual bool HasRotDof() const
    {
        std::cout << "HASROTDOFFFFFFFFFFFFFFFFFFFF\n";
        return (GetGeometry()[0].HasDofFor(ROTATION_Z) && GetGeometry().size() == 2); //Z????
    }

    /**
     * @brief This method computes the DoF block size
     * @return The size of the DoF block
     */
    unsigned int GetBlockSize() const
    {
        unsigned int dim = GetGeometry().WorkingSpaceDimension();
        if( HasRotDof() ) { // if it has rotations
            if(dim == 2)
                return 4;
            else if(dim == 3)
                return 7;
            else
                KRATOS_ERROR << "The conditions only works for 2D and 3D elements";
        } else {
            return dim+1;
        }
    }

    // /**
    //  * @brief Get on rVariable a array_1d Value
    //  * @param rVariable Internal values
    //  * @param rCurrentProcessInfo The current process information
    //  * @param rOutput The values of interest (array_1d)
    //  */
    // void GetValueOnIntegrationPoints(
    //     const Variable<array_1d<double, 3>>& rVariable,
    //     std::vector<array_1d<double, 3>>& rOutput,
    //     const ProcessInfo& rCurrentProcessInfo
    //     ) override;

    // /**
    //  * @brief Calculate a array_1d Variable
    //  * @param rVariable Internal values
    //  * @param rCurrentProcessInfo The current process information
    //  * @param rOutput The values of interest (array_1d)
    //  */
    // void CalculateOnIntegrationPoints(
    //     const Variable<array_1d<double, 3>>& rVariable,
    //     std::vector< array_1d<double, 3>>& rOutput,
    //     const ProcessInfo& rCurrentProcessInfo
    //     ) override;

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "AcousticStructureLineCondition #" << Id();
        return buffer.str();
    }

    /// Print information about this object.

    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "AcousticStructureLineCondition #" << Id();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        pGetGeometry()->PrintData(rOStream);
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

    /**
     * @brief This functions calculates LHS and mass contribution
     * @param rLeftHandSideMatrix: The LHS / mass matrix
     * @param rRightHandSideVector: The RHS
     * @param rCurrentProcessInfo: The current process info instance
     * @param CalculateMassMatrixFlag: The flag to set if to compute mass. otherwise stiffness
     */
    void CalculateAll(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        const ProcessInfo& rCurrentProcessInfo,
        const bool CalculateStiffnessMatrixFlag,
        const bool CalculateMassMatrixFlag,
        const bool CalculateVectorFlag
        ) ;

    /**
     * This functions computes the integration weight to consider
     * @param IntegrationPoints: The array containing the integration points
     * @param PointNumber: The id of the integration point considered
     * @param detJ: The determinant of the jacobian of the element
     */
    virtual double GetIntegrationWeight(
        const GeometryType::IntegrationPointsArrayType& IntegrationPoints,
        const SizeType PointNumber,
        const double detJ
        ) const;

    // /**
    //  * @brief This method adds the local contribution of the pressure to the LHS matrix
    //  * @param rK The local LHS contribution
    //  * @param rTangentXi The axis direction
    //  * @param rDN_De The local gradient of the geometry
    //  * @param rN The shape function of the current integration point
    //  * @param Pressure The pressure to be applied
    //  * @param Weight The integration contribution
    //  */
    // void CalculateAndSubKp(
    //     Matrix& rK,
    //     const array_1d<double, 3>& rTangentXi,
    //     const Matrix& rDN_De,
    //     const Vector& rN,
    //     const double Pressure,
    //     const double IntegrationWeight
    //     ) const;

    // /**
    //  * @brief This method adds the pressure contribution to the RHS
    //  * @param rResidualVector The local contribution to the RHS
    //  * @param rN The corresponding shape function
    //  * @param rNormal The normal to the geometry surface
    //  * @param Pressure The pressure to be applied
    //  * @param Weight The integration contribution
    //  * @param rCurrentProcessInfo The current instance of process info
    //  */
    // void CalculateAndAddPressureForce(
    //     VectorType& rRightHandSideVector,
    //     const Vector& rN,
    //     const array_1d<double, 3>& rNormal,
    //     const double Pressure,
    //     const double IntegrationWeight
    //     ) const;

    /**
     * @brief This method provides the local axis
     * @param rLocalAxis The local axis
     * @param rJacobian The jacobian matrix
     */
    void GetLocalAxis1(
        array_1d<double, 3>& rLocalAxis,
        const Matrix& rJacobian
        ) const;

    /**
     * @brief This method provides the local axis
     * @param rLocalAxis The local axis
     */
    void GetLocalAxis2(array_1d<double, 3>& rLocalAxis) const;

    /**
     * @brief This method provides the cross tangent matrix
     * @param rCrossTangentMatrix The cross tangent matrix
     * @param rTangentXi The axis direction
     */
    void GetCrossTangentMatrix(
        BoundedMatrix<double, TDim, TDim>& rCrossTangentMatrix,
        const array_1d<double, 3>& rTangentXi
        ) const;

    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{

    // A protected default constructor necessary for serialization
    AcousticStructureLineCondition() {};

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save( Serializer& rSerializer ) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, AcousticStructureLineCondition );
    }

    void load( Serializer& rSerializer ) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, AcousticStructureLineCondition );
    }

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    //AcousticStructureLineCondition& operator=(const AcousticStructureLineCondition& rOther);

    /// Copy constructor.
    //AcousticStructureLineCondition(const AcousticStructureLineCondition& rOther);


    ///@}

}; // Class AcousticStructureLineCondition

///@}
///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

/// input stream function
template<std::size_t TDim>
inline std::istream& operator >> (std::istream& rIStream,
        AcousticStructureLineCondition<TDim>& rThis);
/// output stream function
template<std::size_t TDim>
inline std::ostream& operator << (std::ostream& rOStream,
        const AcousticStructureLineCondition<TDim>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

///@}

}  // namespace Kratos.

#endif // KRATOS_ACOUSTIC_STRUCTURE_LINE_CONDITION_H_INCLUDED  defined


