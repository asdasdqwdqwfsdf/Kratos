//
//   Project Name:        KratosSolidMechanicsApplication $
//   Created by:          $Author:            JMCarbonell $
//   Last modified by:    $Co-Author:                     $
//   Date:                $Date:                July 2013 $
//   Revision:            $Revision:                  0.0 $
//
//

// System includes

// External includes

// Project includes
#include "custom_conditions/line_load_condition.hpp"

#include "solid_mechanics_application_variables.h"

namespace Kratos
{

  //***********************************************************************************
  //***********************************************************************************
  LineLoadCondition::LineLoadCondition(IndexType NewId, GeometryType::Pointer pGeometry)
    : LoadCondition(NewId, pGeometry)
  {
    //DO NOT ADD DOFS HERE!!!
  }

  //***********************************************************************************
  //***********************************************************************************
  LineLoadCondition::LineLoadCondition(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
    : LoadCondition(NewId, pGeometry, pProperties)
  {
    //DO NOT ADD DOFS HERE!!!
  }

  //************************************************************************************
  //************************************************************************************
  LineLoadCondition::LineLoadCondition( LineLoadCondition const& rOther )
    : LoadCondition(rOther)     
  {
  }

  //***********************************************************************************
  //***********************************************************************************
  Condition::Pointer LineLoadCondition::Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const
  {
    return Condition::Pointer(new LineLoadCondition(NewId, GetGeometry().Create(ThisNodes), pProperties));
  }


  //************************************CLONE*******************************************
  //************************************************************************************
  Condition::Pointer LineLoadCondition::Clone( IndexType NewId, NodesArrayType const& rThisNodes ) const
  {
  
    LineLoadCondition NewCondition( NewId, GetGeometry().Create( rThisNodes ), pGetProperties() );

    NewCondition.SetData(this->GetData());
    NewCondition.SetFlags(this->GetFlags());

    //-----------//      
    return Condition::Pointer( new LineLoadCondition(NewCondition) );

  }


  //***********************************************************************************
  //***********************************************************************************
  LineLoadCondition::~LineLoadCondition()
  {
  }

  //************* GETTING METHODS

  //************************************************************************************
  //************************************************************************************

  void LineLoadCondition::InitializeGeneralVariables(GeneralVariables& rVariables, const ProcessInfo& rCurrentProcessInfo)
  {
    KRATOS_TRY

    LoadCondition::InitializeGeneralVariables(rVariables, rCurrentProcessInfo);

    //calculating the current jacobian from cartesian coordinates to parent coordinates for all integration points [dx_n+1/d£]
    rVariables.j = GetGeometry().Jacobian( rVariables.j, mThisIntegrationMethod );
  
    //Calculate Delta Position
    //rVariables.DeltaPosition = CalculateDeltaPosition(rVariables.DeltaPosition);

    //calculating the reference jacobian from cartesian coordinates to parent coordinates for all integration points [dx_n/d£]
    //rVariables.J = GetGeometry().Jacobian( rVariables.J, mThisIntegrationMethod, rVariables.DeltaPosition );

    //Calculate Total Delta Position
    rVariables.DeltaPosition = CalculateTotalDeltaPosition(rVariables.DeltaPosition);

    //calculating the reference jacobian from cartesian coordinates to parent coordinates for all integration points [dx_0/d£]
    rVariables.J = GetGeometry().Jacobian( rVariables.J, mThisIntegrationMethod, rVariables.DeltaPosition );

    KRATOS_CATCH( "" )
	  
  }    


  //*************************COMPUTE DELTA POSITION*************************************
  //************************************************************************************


  Matrix& LineLoadCondition::CalculateDeltaPosition(Matrix & rDeltaPosition)
  {
    KRATOS_TRY

    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
    const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();
    
    rDeltaPosition.resize(number_of_nodes , dimension, false);
    rDeltaPosition = zero_matrix<double>( number_of_nodes , dimension);

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
      {
        array_1d<double, 3 > & CurrentDisplacement  = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);
        array_1d<double, 3 > & PreviousDisplacement = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT,1);

        for ( unsigned int j = 0; j < dimension; j++ )
	  {
            rDeltaPosition(i,j) = CurrentDisplacement[j]-PreviousDisplacement[j];
	  }
      }

    return rDeltaPosition;

    KRATOS_CATCH( "" )
  }


  //*************************COMPUTE TOTAL DELTA POSITION*******************************
  //************************************************************************************

  Matrix& LineLoadCondition::CalculateTotalDeltaPosition(Matrix & rDeltaPosition)
  {
    KRATOS_TRY

    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
    const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();

    rDeltaPosition.resize(number_of_nodes , dimension, false);
    rDeltaPosition = zero_matrix<double>( number_of_nodes , dimension);

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
      {
        array_1d<double, 3 > & CurrentDisplacement  = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);
 
        for ( unsigned int j = 0; j < dimension; j++ )
	  {
            rDeltaPosition(i,j) = CurrentDisplacement[j];
	  }
      }

    return rDeltaPosition;

    KRATOS_CATCH( "" )
  }
  
  
  //*********************************COMPUTE KINEMATICS*********************************
  //************************************************************************************

  void LineLoadCondition::CalculateKinematics(GeneralVariables& rVariables,
						const double& rPointNumber)
  {
    KRATOS_TRY

    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
      
    //Get the parent coodinates derivative [dN/d£]
    const GeometryType::ShapeFunctionsGradientsType& DN_De = rVariables.GetShapeFunctionsGradients();
    
    //Get the shape functions for the order of the integration method [N]
    const Matrix& Ncontainer = rVariables.GetShapeFunctions();

    //get first vector of the plane
    rVariables.Tangent1[0] = rVariables.j[rPointNumber](0, 0); // x_1,e
    rVariables.Tangent1[1] = rVariables.j[rPointNumber](1, 0); // x_2,e
   
    rVariables.Normal[0] = -rVariables.j[rPointNumber](1, 0); //-x_2,e
    rVariables.Normal[1] =  rVariables.j[rPointNumber](0, 0); // x_1,e

    if(dimension==3){
      rVariables.Tangent1[2] = rVariables.J[rPointNumber](2, 0); // x_3,e
      rVariables.Normal[2]   = rVariables.J[rPointNumber](2, 0); // x_3,e
    }
    
    //Jacobian to the deformed configuration
    rVariables.Jacobian = norm_2(rVariables.Normal);

    //std::cout<< " jacobian "<<rVariables.Jacobian<<std::endl;

    //Compute the unit normal and weighted tangents
    if(rVariables.Jacobian>0){
      rVariables.Normal   /= rVariables.Jacobian;
      rVariables.Tangent1 /= rVariables.Jacobian;
    }

    //Jacobian to the last known configuration
    rVariables.Tangent2[0] = rVariables.J[rPointNumber](0, 0); // x_1,e
    rVariables.Tangent2[1] = rVariables.J[rPointNumber](1, 0); // x_2,e
    if(dimension==3){
      rVariables.Tangent2[2] = rVariables.J[rPointNumber](2, 0); // x_3,e
    }
 
    rVariables.Jacobian = norm_2(rVariables.Tangent2);

    //Set Shape Functions Values for this integration point
    rVariables.N =row( Ncontainer, rPointNumber);

    //Set Shape Functions Derivatives [dN/d£] for this integration point
    rVariables.DN_De = DN_De[rPointNumber];

    //Get domain size
    rVariables.DomainSize = GetGeometry().Length();
    
    KRATOS_CATCH( "" )
  }


  //***********************************************************************************
  //***********************************************************************************

  Vector& LineLoadCondition::CalculateVectorForce(Vector& rVectorForce, GeneralVariables& rVariables)
  {

    KRATOS_TRY

    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
    const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();
    
    if( rVectorForce.size() != dimension )
      rVectorForce.resize(dimension,false);

    noalias(rVectorForce) = ZeroVector(dimension);
    
    //PRESSURE CONDITION:
    rVectorForce = rVariables.Normal;
    rVariables.Pressure = 0.0;

    //defined on condition
    if( this->Has( NEGATIVE_FACE_PRESSURE ) ){
      double& NegativeFacePressure = this->GetValue( NEGATIVE_FACE_PRESSURE );
      for ( unsigned int i = 0; i < number_of_nodes; i++ )
	rVariables.Pressure += rVariables.N[i] * NegativeFacePressure;
    }

    if( this->Has( POSITIVE_FACE_PRESSURE ) ){
      double& PositiveFacePressure = this->GetValue( POSITIVE_FACE_PRESSURE );
      for ( unsigned int i = 0; i < number_of_nodes; i++ )
	rVariables.Pressure -= rVariables.N[i] * PositiveFacePressure;
    }

    //defined on condition nodes
    for ( unsigned int i = 0; i < number_of_nodes; i++ )
      {
	if( GetGeometry()[i].SolutionStepsDataHas( NEGATIVE_FACE_PRESSURE) ) 
	  rVariables.Pressure += rVariables.N[i] * ( GetGeometry()[i].FastGetSolutionStepValue( NEGATIVE_FACE_PRESSURE ) );
	if( GetGeometry()[i].SolutionStepsDataHas( POSITIVE_FACE_PRESSURE) ) 
	  rVariables.Pressure -= rVariables.N[i] * ( GetGeometry()[i].FastGetSolutionStepValue( POSITIVE_FACE_PRESSURE ) );     
      }
    
    rVectorForce *= rVariables.Pressure;
   
    //FORCE CONDITION:
    
    //defined on condition
    if( this->Has( LINE_LOAD ) ){
      array_1d<double, 3 > & LineLoad = this->GetValue( LINE_LOAD );
      for ( unsigned int i = 0; i < number_of_nodes; i++ )
	{
	  for( unsigned int k = 0; k < dimension; k++ )
	    rVectorForce[k] += rVariables.N[i] * LineLoad[k];
	}
    }

    //defined on condition nodes
    if( this->Has( LINE_LOADS_VECTOR ) ){
      Vector& LineLoads = this->GetValue( LINE_LOADS_VECTOR );
      unsigned int counter = 0;
      for ( unsigned int i = 0; i < number_of_nodes; i++ )
	{
	  counter = i*3;
	  for( unsigned int k = 0; k < dimension; k++ )
	    {
	      rVectorForce[k] += rVariables.N[i] * LineLoads[counter+k];
	    }
	  
	}
    }
    
    //defined on geometry nodes
    for (unsigned int i = 0; i < number_of_nodes; i++)
      {
	if( GetGeometry()[i].SolutionStepsDataHas( LINE_LOAD ) ){
	  array_1d<double, 3 > & LineLoad = GetGeometry()[i].FastGetSolutionStepValue( LINE_LOAD );
	  for( unsigned int k = 0; k < dimension; k++ )
	    rVectorForce[k] += rVariables.N[i] * LineLoad[k];
	}
      }


    return rVectorForce;

    KRATOS_CATCH( "" )
  }

  
  //************* COMPUTING  METHODS
  //************************************************************************************
  //************************************************************************************

  void LineLoadCondition::CalculateAndAddKuug(MatrixType& rLeftHandSideMatrix,
						GeneralVariables& rVariables,
						double& rIntegrationWeight)

  {
    KRATOS_TRY

      const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
      
      if( rVariables.Pressure == 0 )
	{
	  unsigned int size = GetGeometry().PointsNumber() * dimension;
	  if(rLeftHandSideMatrix.size1() != size )
	    rLeftHandSideMatrix.resize(size,size,false);
	  noalias(rLeftHandSideMatrix) = ZeroMatrix( size, size );
	}
      else
	{
	  if( dimension == 2 ){
	  
	    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	
	    Matrix Kij     ( 2, 2 );
	    Matrix SkewSymmMatrix( 2, 2 );
	
	    //Compute the K sub matrix
	    SkewSymmMatrix( 0, 0 ) =  0.0;
	    SkewSymmMatrix( 0, 1 ) = -1.0;
	    SkewSymmMatrix( 1, 0 ) = +1.0;
	    SkewSymmMatrix( 1, 1 ) =  0.0;

	    double DiscretePressure=0;
	    unsigned int RowIndex = 0;
	    unsigned int ColIndex = 0;
        
	    for ( unsigned int i = 0; i < number_of_nodes; i++ )
	      {
		RowIndex = i * 2;
	    
		for ( unsigned int j = 0; j < number_of_nodes; j++ )
		  {
		    ColIndex = j * 2;
		
		    DiscretePressure = rVariables.Pressure * rVariables.N[i] * rVariables.DN_De( j, 0 ) * rIntegrationWeight;
		    Kij = DiscretePressure * SkewSymmMatrix;
		
		    MathUtils<double>::AddMatrix( rLeftHandSideMatrix, Kij, RowIndex, ColIndex );
		  }
	      }

	  }
	  else{ //3D line pressure not considered here
	    unsigned int size = GetGeometry().PointsNumber() * dimension;
	    if(rLeftHandSideMatrix.size1() != size )
	      rLeftHandSideMatrix.resize(size,size,false);
	    noalias(rLeftHandSideMatrix) = ZeroMatrix( size, size );
	  }
 
	}
      
    KRATOS_CATCH( "" )
  }


  //***********************************************************************************
  //***********************************************************************************


  int LineLoadCondition::Check( const ProcessInfo& rCurrentProcessInfo )
  {
    return 0;
  }

  //***********************************************************************************
  //***********************************************************************************

  void LineLoadCondition::save( Serializer& rSerializer ) const
  {
    KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, LoadCondition )
  }

  void LineLoadCondition::load( Serializer& rSerializer )
  {
    KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, LoadCondition )
  }


} // Namespace Kratos.
