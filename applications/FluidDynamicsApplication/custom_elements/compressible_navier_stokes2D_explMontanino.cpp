//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Andrea Montanino
//

#include "custom_elements/compressible_navier_stokes.h"

namespace Kratos {

template<>
void CompressibleNavierStokes<2>::EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    unsigned int SpaceDimension = 2;
    unsigned int nScalarVariables = SpaceDimension + 2;
    unsigned int nodesElement = 3;
    unsigned int nNodalVariables  = nodesElement*nScalarVariables;

    if (rResult.size() != nNodalVariables)
        rResult.resize(nNodalVariables, false);         // A che serve sta roba?

    for(unsigned int i=0; i<nodesElement; i++)
    {
       rResult[i*(nScalarVariables)  ]  =  this->GetGeometry()[i].GetDof(DENSITY).EquationId();	// Collect current value of nodal variable into a local vector U?
       rResult[i*(nScalarVariables)+1]  =  this->GetGeometry()[i].GetDof(MOMENTUM_X).EquationId();
       rResult[i*(nScalarVariables)+2]  =  this->GetGeometry()[i].GetDof(MOMENTUM_Y).EquationId();
       rResult[i*(nScalarVariables)+3]  =  this->GetGeometry()[i].GetDof(TOTAL_ENERGY).EquationId();
    }

    KRATOS_CATCH("")
}


template<>
void CompressibleNavierStokes<2>::GetDofList(DofsVectorType& ElementalDofList, ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    unsigned int SpaceDimension = 2;
    unsigned int nScalarVariables = SpaceDimension+2;
    unsigned int nodesElement = 3;
    unsigned int nNodalVariables  = nodesElement*nScalarVariables;

    if (ElementalDofList.size() != nNodalVariables)     // A che serve sta roba ?
        ElementalDofList.resize(nNodalVariables);

    for(unsigned int i=0; i<NumNodes; i++)
    {
        ElementalDofList[i*(nScalarVariables)  ]  =  this->GetGeometry()[i].pGetDof(DENSITY);		// Difference with the operation made before? 
        ElementalDofList[i*(nScalarVariables)+1]  =  this->GetGeometry()[i].pGetDof(MOMENTUM_X);
        ElementalDofList[i*(nScalarVariables)+2]  =  this->GetGeometry()[i].pGetDof(MOMENTUM_Y);
        ElementalDofList[i*(nScalarVariables)+3]  =  this->GetGeometry()[i].pGetDof(TOTAL_ENERGY);
    }

    KRATOS_CATCH("");
}


template<>
void CompressibleNavierStokes<2>::ComputeGaussPointLHSContribution(BoundedMatrix<double,12,12>& lhs, const ElementDataStruct& data,double data_v_sc, double data_k_sc)
{

    const int nnodes = 3;				// OK
    const int dim = 2;					// OK
    const int BlockSize = dim+2;			// OK
    const double h = data.h; 				// Which is the element size? In my case is the radius of the inscribed circle

    const double& bdf0 = data.bdf0;		// To be computed according to the time discretization (in RK4 may be different among the different substeps !!)
    const double& bdf1 = data.bdf1;	
    const double& bdf2 = data.bdf2;
    
// From where are taken the variables U, Un ? Why it is a matrix? I prefer a vector 

    const BoundedMatrix<double,nnodes,BlockSize>& U = data.U;	// Current step
    const BoundedMatrix<double,nnodes,BlockSize>& Un = data.Un;	//  step n-1 (right?)
    const BoundedMatrix<double,nnodes,BlockSize>& Unn = data.Unn;	// step n-2 (right?)
    const BoundedMatrix<double,nnodes,dim>& f_ext = data.f_ext;		// body force
    const array_1d<double,nnodes>& r = data.r;				// heat sorce (energy source)
    const double mu = data.mu;						// OK
    const double nu = data.nu;						// Which nu? Isn't mu/ro?
    const double lambda = data.lambda;					// OK
    const double c_v = data.c_v;					// OK
    const double gamma = data.gamma;					// OK
    const double cp = c_v*gamma;					// OK
    const double v_sc = data_v_sc;					// Computed with shock capturing, right?)
    const double k_sc = data_k_sc;					// Computed with shock capturing, right?)
 
    // Get shape function values
    const array_1d<double,nnodes>& N = data.N;				// Evaluation of N (in which point? Where is it specified?)
    const BoundedMatrix<double,nnodes,dim>& DN = data.DN_DX;		// Evaluation of the gradient matrix (in which point? Where is it specified?)
    
    // Stabilization parameters
    const double stab_c1 = 4.0;						// OK
    const double stab_c2 = 2.0;						// OK
    
    const array_1d<double,BlockSize> U_gauss= prod(trans(U),N);		// Evalutaion of U in the gauss point (?) 
    
    double tmp = U_gauss(dim+1)/U_gauss(0);
    for(unsigned int ll=0; ll<dim; ll++)
        tmp -=(U_gauss(ll+1)*U_gauss(ll+1))/(2*U_gauss(0)*U_gauss(0));  // Speed of the sound in the Gauss Point 
    double c = sqrt(gamma*(gamma-1)*tmp);

    double tau1inv = 0.0;
    for(unsigned int ll=0; ll<dim; ll++)					// Definition of tau_stab
        tau1inv += (U_gauss(ll+1)/U_gauss(0))*(U_gauss(ll+1)/U_gauss(0));
    tau1inv = (sqrt(tau1inv)+c)*stab_c2/h;
    double tau2inv = stab_c1*nu/(h*h)+tau1inv;
    double tau3inv = stab_c1*lambda/(U_gauss(0)*cp*h*h)+tau1inv;
        
    const double tau1 = 1/tau1inv;
    const double tau2 = 1/tau2inv;
    const double tau3 = 1/tau3inv;

// From here, terrore, morte y devastaciòn

/*
    const double clhs0 =             pow(N[0], 2);
const double clhs1 =             -bdf0*clhs0;
const double clhs2 =             (1.0L/2.0L)*DN(0,0)*tau2;
const double clhs3 =             2*N[0];
const double clhs4 =             N[0]*f_ext(0,0) + N[1]*f_ext(1,0) + N[2]*f_ext(2,0);
const double clhs5 =             clhs3*clhs4;
const double clhs6 =             N[0]*U(0,0) + N[1]*U(1,0) + N[2]*U(2,0);
const double clhs7 =             pow(clhs6, -2);
const double clhs8 =             N[0]*U(0,2) + N[1]*U(1,2) + N[2]*U(2,2);
const double clhs9 =             2*DN(0,1);
const double clhs10 =             N[0]*U(0,1) + N[1]*U(1,1) + N[2]*U(2,1);
const double clhs11 =             clhs10*clhs9;
const double clhs12 =             clhs11*clhs8;
const double clhs13 =             clhs12*clhs7;
const double clhs14 =             DN(0,1)*U(0,1);
const double clhs15 =             DN(1,1)*U(1,1);
const double clhs16 =             DN(2,1)*U(2,1);
const double clhs17 =             clhs14 + clhs15 + clhs16;
const double clhs18 =             2*N[0]*U(0,2) + 2*N[1]*U(1,2) + 2*N[2]*U(2,2);
const double clhs19 =             N[0]*clhs17*clhs18*clhs7;
const double clhs20 =             DN(0,1)*U(0,2) + DN(1,1)*U(1,2) + DN(2,1)*U(2,2);
const double clhs21 =             N[0]*clhs20;
const double clhs22 =             2*clhs21;
const double clhs23 =             clhs10*clhs22;
const double clhs24 =             clhs23*clhs7;
const double clhs25 =             DN(0,0)*U(0,1);
const double clhs26 =             DN(1,0)*U(1,1);
const double clhs27 =             DN(2,0)*U(2,1);
const double clhs28 =             clhs25 + clhs26 + clhs27;
const double clhs29 =             N[0]*clhs28;
const double clhs30 =             2*clhs29;
const double clhs31 =             gamma - 3;
const double clhs32 =             clhs10*clhs31;
const double clhs33 =             clhs30*clhs32;
const double clhs34 =             clhs33*clhs7;
const double clhs35 =             -clhs34;
const double clhs36 =             2*gamma - 2;
const double clhs37 =             DN(0,0)*U(0,2);
const double clhs38 =             DN(1,0)*U(1,2);
const double clhs39 =             DN(2,0)*U(2,2);
const double clhs40 =             clhs37 + clhs38 + clhs39;
const double clhs41 =             N[0]*clhs36*clhs40*clhs8;
const double clhs42 =             clhs41*clhs7;
const double clhs43 =             -clhs42;
const double clhs44 =             pow(clhs6, -3);
const double clhs45 =             N[0]*clhs44;
const double clhs46 =             DN(0,1)*U(0,0) + DN(1,1)*U(1,0) + DN(2,1)*U(2,0);
const double clhs47 =             4*N[0]*U(0,2) + 4*N[1]*U(1,2) + 4*N[2]*U(2,2);
const double clhs48 =             clhs10*clhs46*clhs47;
const double clhs49 =             clhs45*clhs48;
const double clhs50 =             -clhs49;
const double clhs51 =             pow(clhs10, 2);
const double clhs52 =             2*clhs51;
const double clhs53 =             gamma - 1;
const double clhs54 =             clhs51*clhs53;
const double clhs55 =             pow(clhs8, 2);
const double clhs56 =             clhs53*clhs55;
const double clhs57 =             clhs54 + clhs56;
const double clhs58 =             -clhs52 + clhs57;
const double clhs59 =             DN(0,0)*clhs58;
const double clhs60 =             clhs59*clhs7;
const double clhs61 =             -clhs60;
const double clhs62 =             DN(0,0)*U(0,0) + DN(1,0)*U(1,0) + DN(2,0)*U(2,0);
const double clhs63 =             clhs44*clhs62;
const double clhs64 =             2*N[0]*clhs58;
const double clhs65 =             clhs63*clhs64;
const double clhs66 =             clhs13 + clhs19 + clhs24 + clhs35 + clhs43 + clhs50 + clhs61 + clhs65;
const double clhs67 =             clhs5 + clhs66;
const double clhs68 =             (1.0L/2.0L)*DN(0,1)*tau2;
const double clhs69 =             N[0]*f_ext(0,1) + N[1]*f_ext(1,1) + N[2]*f_ext(2,1);
const double clhs70 =             clhs3*clhs69;
const double clhs71 =             2*DN(0,0);
const double clhs72 =             clhs10*clhs71;
const double clhs73 =             clhs72*clhs8;
const double clhs74 =             clhs7*clhs73;
const double clhs75 =             clhs30*clhs8;
const double clhs76 =             clhs7*clhs75;
const double clhs77 =             2*N[0]*U(0,1) + 2*N[1]*U(1,1) + 2*N[2]*U(2,1);
const double clhs78 =             N[0]*clhs40*clhs7*clhs77;
const double clhs79 =             clhs31*clhs8;
const double clhs80 =             clhs22*clhs79;
const double clhs81 =             clhs7*clhs80;
const double clhs82 =             -clhs81;
const double clhs83 =             N[0]*clhs10*clhs17*clhs36;
const double clhs84 =             clhs7*clhs83;
const double clhs85 =             -clhs84;
const double clhs86 =             4*N[0]*U(0,1) + 4*N[1]*U(1,1) + 4*N[2]*U(2,1);
const double clhs87 =             clhs62*clhs8*clhs86;
const double clhs88 =             clhs45*clhs87;
const double clhs89 =             -clhs88;
const double clhs90 =             2*clhs55;
const double clhs91 =             clhs57 - clhs90;
const double clhs92 =             DN(0,1)*clhs91;
const double clhs93 =             clhs7*clhs92;
const double clhs94 =             -clhs93;
const double clhs95 =             clhs44*clhs46;
const double clhs96 =             2*N[0]*clhs91;
const double clhs97 =             clhs95*clhs96;
const double clhs98 =             clhs74 + clhs76 + clhs78 + clhs82 + clhs85 + clhs89 + clhs94 + clhs97;
const double clhs99 =             clhs70 + clhs98;
const double clhs100 =             DN(0,0)*N[0];
const double clhs101 =             -clhs100;
const double clhs102 =             1.0/clhs6;
const double clhs103 =             DN(0,1)*clhs102*tau2;
const double clhs104 =             DN(0,0)*clhs8;
const double clhs105 =             N[0]*clhs40;
const double clhs106 =             DN(0,1)*clhs10;
const double clhs107 =             clhs106*clhs53;
const double clhs108 =             N[0]*clhs17;
const double clhs109 =             clhs108*clhs53;
const double clhs110 =             -clhs109;
const double clhs111 =             N[0]*clhs102;
const double clhs112 =             clhs62*clhs8;
const double clhs113 =             clhs111*clhs112;
const double clhs114 =             clhs10*clhs46;
const double clhs115 =             clhs111*clhs114;
const double clhs116 =             clhs104 + clhs105 - clhs107 + clhs110 - clhs113 + clhs115*clhs53;
const double clhs117 =             DN(0,0)*tau2;
const double clhs118 =             N[0]*bdf0;
const double clhs119 =             -clhs118;
const double clhs120 =             DN(0,1)*clhs8;
const double clhs121 =             clhs102*clhs120;
const double clhs122 =             clhs102*clhs21;
const double clhs123 =             DN(0,0)*clhs10;
const double clhs124 =             clhs123*clhs31;
const double clhs125 =             clhs102*clhs124;
const double clhs126 =             clhs29*clhs31;
const double clhs127 =             clhs102*clhs126;
const double clhs128 =             N[0]*clhs7;
const double clhs129 =             clhs46*clhs8;
const double clhs130 =             clhs128*clhs129;
const double clhs131 =             clhs10*clhs31*clhs62;
const double clhs132 =             clhs128*clhs131;
const double clhs133 =             clhs119 - clhs121 - clhs122 + clhs125 + clhs127 + clhs130 - clhs132;
const double clhs134 =             DN(0,1)*N[0];
const double clhs135 =             -clhs134;
const double clhs136 =             DN(0,0)*clhs102*tau2;
const double clhs137 =             clhs105*clhs53;
const double clhs138 =             -clhs108 + clhs137;
const double clhs139 =             clhs104*clhs53 - clhs106 - clhs113*clhs53 + clhs115 + clhs138;
const double clhs140 =             DN(0,1)*tau2;
const double clhs141 =             clhs102*clhs123;
const double clhs142 =             clhs102*clhs29;
const double clhs143 =             clhs120*clhs31;
const double clhs144 =             clhs102*clhs143;
const double clhs145 =             clhs21*clhs31;
const double clhs146 =             clhs102*clhs145;
const double clhs147 =             clhs10*clhs62;
const double clhs148 =             clhs128*clhs147;
const double clhs149 =             clhs31*clhs46*clhs8;
const double clhs150 =             clhs128*clhs149;
const double clhs151 =             clhs118 + clhs141 + clhs142 - clhs144 - clhs146 - clhs148 + clhs150;
const double clhs152 =             clhs53*tau2;
const double clhs153 =             -N[1]*clhs118;
const double clhs154 =             2*N[1];
const double clhs155 =             clhs154*clhs4;
const double clhs156 =             2*DN(1,1);
const double clhs157 =             clhs10*clhs156;
const double clhs158 =             clhs157*clhs8;
const double clhs159 =             clhs158*clhs7;
const double clhs160 =             N[1]*clhs17*clhs18*clhs7;
const double clhs161 =             N[1]*clhs20;
const double clhs162 =             2*clhs161;
const double clhs163 =             clhs10*clhs162;
const double clhs164 =             clhs163*clhs7;
const double clhs165 =             N[1]*clhs28;
const double clhs166 =             2*clhs165;
const double clhs167 =             clhs166*clhs32;
const double clhs168 =             clhs167*clhs7;
const double clhs169 =             -clhs168;
const double clhs170 =             N[1]*clhs36*clhs40*clhs8;
const double clhs171 =             clhs170*clhs7;
const double clhs172 =             -clhs171;
const double clhs173 =             N[1]*clhs44;
const double clhs174 =             clhs173*clhs48;
const double clhs175 =             -clhs174;
const double clhs176 =             DN(1,0)*clhs58;
const double clhs177 =             clhs176*clhs7;
const double clhs178 =             -clhs177;
const double clhs179 =             2*N[1]*clhs58;
const double clhs180 =             clhs179*clhs63;
const double clhs181 =             clhs159 + clhs160 + clhs164 + clhs169 + clhs172 + clhs175 + clhs178 + clhs180;
const double clhs182 =             clhs155 + clhs181;
const double clhs183 =             clhs154*clhs69;
const double clhs184 =             2*DN(1,0);
const double clhs185 =             clhs10*clhs184;
const double clhs186 =             clhs185*clhs8;
const double clhs187 =             clhs186*clhs7;
const double clhs188 =             clhs166*clhs8;
const double clhs189 =             clhs188*clhs7;
const double clhs190 =             N[1]*clhs40*clhs7*clhs77;
const double clhs191 =             clhs162*clhs79;
const double clhs192 =             clhs191*clhs7;
const double clhs193 =             -clhs192;
const double clhs194 =             N[1]*clhs10*clhs17*clhs36;
const double clhs195 =             clhs194*clhs7;
const double clhs196 =             -clhs195;
const double clhs197 =             clhs173*clhs87;
const double clhs198 =             -clhs197;
const double clhs199 =             DN(1,1)*clhs91;
const double clhs200 =             clhs199*clhs7;
const double clhs201 =             -clhs200;
const double clhs202 =             2*N[1]*clhs91;
const double clhs203 =             clhs202*clhs95;
const double clhs204 =             clhs187 + clhs189 + clhs190 + clhs193 + clhs196 + clhs198 + clhs201 + clhs203;
const double clhs205 =             clhs183 + clhs204;
const double clhs206 =             DN(1,0)*N[0];
const double clhs207 =             -clhs206;
const double clhs208 =             DN(1,0)*clhs8;
const double clhs209 =             N[1]*clhs40;
const double clhs210 =             DN(1,1)*clhs10;
const double clhs211 =             clhs210*clhs53;
const double clhs212 =             N[1]*clhs17;
const double clhs213 =             clhs212*clhs53;
const double clhs214 =             -clhs213;
const double clhs215 =             N[1]*clhs102;
const double clhs216 =             clhs112*clhs215;
const double clhs217 =             clhs114*clhs215;
const double clhs218 =             clhs208 + clhs209 - clhs211 + clhs214 - clhs216 + clhs217*clhs53;
const double clhs219 =             N[1]*bdf0;
const double clhs220 =             -clhs219;
const double clhs221 =             DN(1,1)*clhs8;
const double clhs222 =             clhs102*clhs221;
const double clhs223 =             clhs102*clhs161;
const double clhs224 =             DN(1,0)*clhs10;
const double clhs225 =             clhs224*clhs31;
const double clhs226 =             clhs102*clhs225;
const double clhs227 =             clhs165*clhs31;
const double clhs228 =             clhs102*clhs227;
const double clhs229 =             N[1]*clhs7;
const double clhs230 =             clhs129*clhs229;
const double clhs231 =             clhs131*clhs229;
const double clhs232 =             clhs220 - clhs222 - clhs223 + clhs226 + clhs228 + clhs230 - clhs231;
const double clhs233 =             DN(1,1)*N[0];
const double clhs234 =             -clhs233;
const double clhs235 =             clhs209*clhs53;
const double clhs236 =             -clhs212 + clhs235;
const double clhs237 =             clhs208*clhs53 - clhs210 - clhs216*clhs53 + clhs217 + clhs236;
const double clhs238 =             clhs102*clhs224;
const double clhs239 =             clhs102*clhs165;
const double clhs240 =             clhs221*clhs31;
const double clhs241 =             clhs102*clhs240;
const double clhs242 =             clhs161*clhs31;
const double clhs243 =             clhs102*clhs242;
const double clhs244 =             clhs147*clhs229;
const double clhs245 =             clhs149*clhs229;
const double clhs246 =             clhs219 + clhs238 + clhs239 - clhs241 - clhs243 - clhs244 + clhs245;
const double clhs247 =             -clhs152*(DN(0,0)*DN(1,0) + DN(0,1)*DN(1,1));
const double clhs248 =             -N[2]*clhs118;
const double clhs249 =             2*N[2];
const double clhs250 =             clhs249*clhs4;
const double clhs251 =             2*DN(2,1);
const double clhs252 =             clhs10*clhs251;
const double clhs253 =             clhs252*clhs8;
const double clhs254 =             clhs253*clhs7;
const double clhs255 =             N[2]*clhs17*clhs18*clhs7;
const double clhs256 =             N[2]*clhs20;
const double clhs257 =             2*clhs256;
const double clhs258 =             clhs10*clhs257;
const double clhs259 =             clhs258*clhs7;
const double clhs260 =             N[2]*clhs28;
const double clhs261 =             2*clhs260;
const double clhs262 =             clhs261*clhs32;
const double clhs263 =             clhs262*clhs7;
const double clhs264 =             -clhs263;
const double clhs265 =             N[2]*clhs36*clhs40*clhs8;
const double clhs266 =             clhs265*clhs7;
const double clhs267 =             -clhs266;
const double clhs268 =             N[2]*clhs44;
const double clhs269 =             clhs268*clhs48;
const double clhs270 =             -clhs269;
const double clhs271 =             DN(2,0)*clhs58;
const double clhs272 =             clhs271*clhs7;
const double clhs273 =             -clhs272;
const double clhs274 =             2*N[2]*clhs58;
const double clhs275 =             clhs274*clhs63;
const double clhs276 =             clhs254 + clhs255 + clhs259 + clhs264 + clhs267 + clhs270 + clhs273 + clhs275;
const double clhs277 =             clhs250 + clhs276;
const double clhs278 =             clhs249*clhs69;
const double clhs279 =             2*DN(2,0);
const double clhs280 =             clhs10*clhs279;
const double clhs281 =             clhs280*clhs8;
const double clhs282 =             clhs281*clhs7;
const double clhs283 =             clhs261*clhs8;
const double clhs284 =             clhs283*clhs7;
const double clhs285 =             N[2]*clhs40*clhs7*clhs77;
const double clhs286 =             clhs257*clhs79;
const double clhs287 =             clhs286*clhs7;
const double clhs288 =             -clhs287;
const double clhs289 =             N[2]*clhs10*clhs17*clhs36;
const double clhs290 =             clhs289*clhs7;
const double clhs291 =             -clhs290;
const double clhs292 =             clhs268*clhs87;
const double clhs293 =             -clhs292;
const double clhs294 =             DN(2,1)*clhs91;
const double clhs295 =             clhs294*clhs7;
const double clhs296 =             -clhs295;
const double clhs297 =             2*N[2]*clhs91;
const double clhs298 =             clhs297*clhs95;
const double clhs299 =             clhs282 + clhs284 + clhs285 + clhs288 + clhs291 + clhs293 + clhs296 + clhs298;
const double clhs300 =             clhs278 + clhs299;
const double clhs301 =             DN(2,0)*N[0];
const double clhs302 =             -clhs301;
const double clhs303 =             DN(2,0)*clhs8;
const double clhs304 =             N[2]*clhs40;
const double clhs305 =             DN(2,1)*clhs10;
const double clhs306 =             clhs305*clhs53;
const double clhs307 =             N[2]*clhs17;
const double clhs308 =             clhs307*clhs53;
const double clhs309 =             -clhs308;
const double clhs310 =             N[2]*clhs102;
const double clhs311 =             clhs112*clhs310;
const double clhs312 =             clhs114*clhs310;
const double clhs313 =             clhs303 + clhs304 - clhs306 + clhs309 - clhs311 + clhs312*clhs53;
const double clhs314 =             N[2]*bdf0;
const double clhs315 =             -clhs314;
const double clhs316 =             DN(2,1)*clhs8;
const double clhs317 =             clhs102*clhs316;
const double clhs318 =             clhs102*clhs256;
const double clhs319 =             DN(2,0)*clhs10;
const double clhs320 =             clhs31*clhs319;
const double clhs321 =             clhs102*clhs320;
const double clhs322 =             clhs260*clhs31;
const double clhs323 =             clhs102*clhs322;
const double clhs324 =             N[2]*clhs7;
const double clhs325 =             clhs129*clhs324;
const double clhs326 =             clhs131*clhs324;
const double clhs327 =             clhs315 - clhs317 - clhs318 + clhs321 + clhs323 + clhs325 - clhs326;
const double clhs328 =             DN(2,1)*N[0];
const double clhs329 =             -clhs328;
const double clhs330 =             clhs304*clhs53;
const double clhs331 =             -clhs307 + clhs330;
const double clhs332 =             clhs303*clhs53 - clhs305 - clhs311*clhs53 + clhs312 + clhs331;
const double clhs333 =             clhs102*clhs319;
const double clhs334 =             clhs102*clhs260;
const double clhs335 =             clhs31*clhs316;
const double clhs336 =             clhs102*clhs335;
const double clhs337 =             clhs256*clhs31;
const double clhs338 =             clhs102*clhs337;
const double clhs339 =             clhs147*clhs324;
const double clhs340 =             clhs149*clhs324;
const double clhs341 =             clhs314 + clhs333 + clhs334 - clhs336 - clhs338 - clhs339 + clhs340;
const double clhs342 =             -clhs152*(DN(0,0)*DN(2,0) + DN(0,1)*DN(2,1));
const double clhs343 =             clhs0*clhs4;
const double clhs344 =             clhs112 + clhs114;
const double clhs345 =             clhs102*clhs344 - clhs14 - clhs15 - clhs16 - clhs37 - clhs38 - clhs39;
const double clhs346 =             clhs102*clhs345*v_sc;
const double clhs347 =             clhs102*clhs62;
const double clhs348 =             clhs102*clhs8;
const double clhs349 =             clhs348*clhs46;
const double clhs350 =             -U(0,1)*clhs71 - U(1,1)*clhs184 - U(2,1)*clhs279 + clhs20 + clhs347*clhs77 - clhs349;
const double clhs351 =             (2.0L/3.0L)*clhs102*clhs350*v_sc;
const double clhs352 =             clhs6*v_sc/mu + 1;
const double clhs353 =             DN(0,1)*clhs352*clhs7*mu;
const double clhs354 =             clhs17 + clhs40;
const double clhs355 =             2*N[0]*clhs102;
const double clhs356 =             N[0]*clhs354 + clhs104 + clhs106 - clhs344*clhs355;
const double clhs357 =             (2.0L/3.0L)*DN(0,0)*clhs352*clhs7*mu;
const double clhs358 =             -clhs21;
const double clhs359 =             -clhs120 + clhs358;
const double clhs360 =             clhs111*clhs62;
const double clhs361 =             clhs10*clhs360;
const double clhs362 =             clhs3*clhs349;
const double clhs363 =             clhs30 + clhs359 - 4*clhs361 + clhs362 + clhs72;
const double clhs364 =             (1.0L/2.0L)*N[0]*clhs7;
const double clhs365 =             clhs108*clhs18;
const double clhs366 =             clhs36*clhs8;
const double clhs367 =             clhs105*clhs366;
const double clhs368 =             clhs115*clhs47 - clhs12 - clhs23 + clhs33 - clhs347*clhs64 - clhs365 + clhs367 + clhs59;
const double clhs369 =             (1.0L/2.0L)*N[0]*bdf0*tau1;
const double clhs370 =             -clhs13 - clhs19 - clhs24 + clhs34 + clhs42 + clhs49 + clhs60;
const double clhs371 =             clhs370 + clhs5 - clhs65;
const double clhs372 =             N[0]*(U(0,0)*bdf0 + Un(0,0)*bdf1 + Unn(0,0)*bdf2) + N[1]*(U(1,0)*bdf0 + Un(1,0)*bdf1 + Unn(1,0)*bdf2) + N[2]*(U(2,0)*bdf0 + Un(2,0)*bdf1 + Unn(2,0)*bdf2) + clhs20 + clhs25 + clhs26 + clhs27;
const double clhs373 =             N[0]*clhs372*clhs44*tau1;
const double clhs374 =             6*N[0]*U(0,2) + 6*N[1]*U(1,2) + 6*N[2]*U(2,2);
const double clhs375 =             -clhs54 - clhs56;
const double clhs376 =             clhs375 + clhs52;
const double clhs377 =             3*clhs51*clhs53;
const double clhs378 =             3*clhs53*clhs55;
const double clhs379 =             clhs377 + clhs378 - 6*clhs51;
const double clhs380 =             clhs8*clhs9;
const double clhs381 =             -clhs380;
const double clhs382 =             clhs31*clhs72;
const double clhs383 =             clhs10*clhs102*clhs31*clhs62;
const double clhs384 =             clhs3*clhs383;
const double clhs385 =             clhs3*(U(0,1)*bdf0 + Un(0,1)*bdf1 + Unn(0,1)*bdf2);
const double clhs386 =             clhs154*(U(1,1)*bdf0 + Un(1,1)*bdf1 + Unn(1,1)*bdf2);
const double clhs387 =             clhs249*(U(2,1)*bdf0 + Un(2,1)*bdf1 + Unn(2,1)*bdf2);
const double clhs388 =             DN(0,0)*U(0,3);
const double clhs389 =             DN(1,0)*U(1,3);
const double clhs390 =             DN(2,0)*U(2,3);
const double clhs391 =             clhs388 + clhs389 + clhs390;
const double clhs392 =             clhs36*clhs391;
const double clhs393 =             2*N[0]*U(0,0) + 2*N[1]*U(1,0) + 2*N[2]*U(2,0);
const double clhs394 =             clhs393*clhs4;
const double clhs395 =             clhs102*clhs18;
const double clhs396 =             clhs17*clhs395;
const double clhs397 =             clhs102*clhs77;
const double clhs398 =             clhs20*clhs397;
const double clhs399 =             clhs28*clhs31*clhs397;
const double clhs400 =             clhs102*clhs36*clhs8;
const double clhs401 =             clhs40*clhs400;
const double clhs402 =             clhs114*clhs18*clhs7;
const double clhs403 =             clhs62*clhs7;
const double clhs404 =             clhs403*clhs58;
const double clhs405 =             clhs385 + clhs386 + clhs387 + clhs392 - clhs394 + clhs396 + clhs398 - clhs399 - clhs401 - clhs402 + clhs404;
const double clhs406 =             (1.0L/2.0L)*N[0]*clhs405*clhs7*tau2;
const double clhs407 =             clhs71*clhs8;
const double clhs408 =             clhs114*clhs355;
const double clhs409 =             clhs112*clhs355;
const double clhs410 =             -clhs11 + clhs138 + clhs407*clhs53 + clhs408 - clhs409*clhs53;
const double clhs411 =             clhs3*(U(0,2)*bdf0 + Un(0,2)*bdf1 + Unn(0,2)*bdf2);
const double clhs412 =             clhs154*(U(1,2)*bdf0 + Un(1,2)*bdf1 + Unn(1,2)*bdf2);
const double clhs413 =             clhs249*(U(2,2)*bdf0 + Un(2,2)*bdf1 + Unn(2,2)*bdf2);
const double clhs414 =             DN(0,1)*U(0,3);
const double clhs415 =             DN(1,1)*U(1,3);
const double clhs416 =             DN(2,1)*U(2,3);
const double clhs417 =             clhs414 + clhs415 + clhs416;
const double clhs418 =             clhs36*clhs417;
const double clhs419 =             clhs393*clhs69;
const double clhs420 =             clhs28*clhs395;
const double clhs421 =             clhs397*clhs40;
const double clhs422 =             clhs20*clhs31*clhs395;
const double clhs423 =             clhs10*clhs102*clhs36;
const double clhs424 =             clhs17*clhs423;
const double clhs425 =             clhs62*clhs7*clhs77*clhs8;
const double clhs426 =             clhs46*clhs7;
const double clhs427 =             clhs426*clhs91;
const double clhs428 =             clhs411 + clhs412 + clhs413 + clhs418 - clhs419 + clhs420 + clhs421 - clhs422 - clhs424 - clhs425 + clhs427;
const double clhs429 =             (1.0L/2.0L)*N[0]*clhs428*clhs7*tau2;
const double clhs430 =             clhs111*clhs46;
const double clhs431 =             clhs430*clhs8;
const double clhs432 =             clhs32*clhs360;
const double clhs433 =             clhs124 + clhs126 + clhs359 + clhs431 - clhs432;
const double clhs434 =             (1.0L/2.0L)*clhs102*clhs433*tau2;
const double clhs435 =             -clhs5;
const double clhs436 =             clhs376*clhs44*clhs62;
const double clhs437 =             clhs3*clhs436;
const double clhs438 =             clhs370 + clhs435 + clhs437;
const double clhs439 =             (1.0L/2.0L)*clhs102*clhs139*tau2;
const double clhs440 =             -clhs70;
const double clhs441 =             clhs375 + clhs90;
const double clhs442 =             clhs44*clhs441*clhs46;
const double clhs443 =             clhs3*clhs442;
const double clhs444 =             -clhs74 - clhs76 - clhs78 + clhs81 + clhs84 + clhs88 + clhs93;
const double clhs445 =             clhs440 + clhs443 + clhs444;
const double clhs446 =             (1.0L/2.0L)*DN(0,0)*clhs53*tau3;
const double clhs447 =             N[0]*r[0] + N[1]*r[1] + N[2]*r[2];
const double clhs448 =             clhs3*clhs447;
const double clhs449 =             -clhs448;
const double clhs450 =             N[0]*clhs391;
const double clhs451 =             clhs450*gamma;
const double clhs452 =             2*clhs451;
const double clhs453 =             clhs10*clhs452;
const double clhs454 =             clhs453*clhs7;
const double clhs455 =             N[0]*clhs417;
const double clhs456 =             clhs455*gamma;
const double clhs457 =             2*clhs456;
const double clhs458 =             clhs457*clhs8;
const double clhs459 =             clhs458*clhs7;
const double clhs460 =             clhs10*clhs44;
const double clhs461 =             N[0]*clhs40*clhs47*clhs53;
const double clhs462 =             clhs460*clhs461;
const double clhs463 =             N[0]*clhs17*clhs47*clhs53;
const double clhs464 =             clhs460*clhs463;
const double clhs465 =             clhs102*clhs56;
const double clhs466 =             N[0]*U(0,3);
const double clhs467 =             2*clhs466;
const double clhs468 =             -clhs467;
const double clhs469 =             N[1]*U(1,3);
const double clhs470 =             2*clhs469;
const double clhs471 =             -clhs470;
const double clhs472 =             N[2]*U(2,3);
const double clhs473 =             2*clhs472;
const double clhs474 =             -clhs473;
const double clhs475 =             clhs466 + clhs469 + clhs472;
const double clhs476 =             clhs36*clhs475;
const double clhs477 =             -clhs476;
const double clhs478 =             clhs102*clhs54;
const double clhs479 =             clhs102*clhs53;
const double clhs480 =             clhs51 + clhs55;
const double clhs481 =             clhs479*clhs480;
const double clhs482 =             clhs468 + clhs471 + clhs474 + clhs477 + clhs478 + clhs481;
const double clhs483 =             clhs465 + clhs482;
const double clhs484 =             clhs123*clhs483;
const double clhs485 =             clhs484*clhs7;
const double clhs486 =             clhs120*clhs483;
const double clhs487 =             clhs486*clhs7;
const double clhs488 =             clhs465 + clhs468 + clhs471 + clhs474 + clhs477;
const double clhs489 =             5*clhs478 + clhs481 + clhs488;
const double clhs490 =             clhs29*clhs489;
const double clhs491 =             clhs490*clhs7;
const double clhs492 =             5*clhs465 + clhs482;
const double clhs493 =             clhs21*clhs492;
const double clhs494 =             clhs493*clhs7;
const double clhs495 =             -clhs478;
const double clhs496 =             -clhs102*clhs36*clhs480;
const double clhs497 =             -clhs465 + clhs467 + clhs470 + clhs473 + clhs476 + clhs496;
const double clhs498 =             clhs495 + clhs497;
const double clhs499 =             2*N[0]*clhs44*clhs498;
const double clhs500 =             clhs147*clhs499;
const double clhs501 =             clhs129*clhs499;
const double clhs502 =             -clhs454 - clhs459 + clhs462 + clhs464 + clhs485 + clhs487 + clhs491 + clhs494 + clhs500 + clhs501;
const double clhs503 =             clhs449 + clhs502;
const double clhs504 =             clhs116*clhs7*tau2;
const double clhs505 =             clhs1 + clhs139*clhs504;
const double clhs506 =             DN(0,0)*clhs102;
const double clhs507 =             -clhs360;
const double clhs508 =             DN(0,0) + clhs507;
const double clhs509 =             clhs352*clhs508*mu;
const double clhs510 =             clhs506*clhs509;
const double clhs511 =             DN(0,1)*clhs102;
const double clhs512 =             -clhs430;
const double clhs513 =             DN(0,1) + clhs512;
const double clhs514 =             clhs352*clhs513*mu;
const double clhs515 =             clhs511*clhs514;
const double clhs516 =             clhs120 + clhs21 - clhs431;
const double clhs517 =             -clhs126;
const double clhs518 =             -clhs124 + clhs432 + clhs516 + clhs517;
const double clhs519 =             N[0]*clhs372*clhs7*tau1;
const double clhs520 =             -clhs362;
const double clhs521 =             clhs21 + clhs380 + clhs520;
const double clhs522 =             clhs102*clhs433*tau2;
const double clhs523 =             (1.0L/2.0L)*DN(0,0)*tau1;
const double clhs524 =             clhs435 + clhs66;
const double clhs525 =             clhs512 + clhs9;
const double clhs526 =             (1.0L/2.0L)*N[0]*clhs102*clhs525*tau2;
const double clhs527 =             -clhs411 - clhs412 - clhs413 - clhs418 + clhs419 - clhs420 - clhs421 + clhs422 + clhs424 + clhs425 - clhs427;
const double clhs528 =             clhs526*clhs527;
const double clhs529 =             clhs507 + clhs71;
const double clhs530 =             (1.0L/2.0L)*N[0]*clhs102*clhs529*tau2;
const double clhs531 =             -clhs385 - clhs386 - clhs387 - clhs392 + clhs394 - clhs396 - clhs398 + clhs399 + clhs401 + clhs402 - clhs404;
const double clhs532 =             clhs530*clhs531;
const double clhs533 =             clhs11*clhs53;
const double clhs534 =             clhs7*clhs8;
const double clhs535 =             6*N[0]*U(0,1) + 6*N[1]*U(1,1) + 6*N[2]*U(2,1);
const double clhs536 =             clhs53*clhs535*clhs7;
const double clhs537 =             N[0]*clhs17*clhs36*clhs8;
const double clhs538 =             3*clhs478;
const double clhs539 =             clhs488 + clhs538;
const double clhs540 =             clhs489*clhs62;
const double clhs541 =             -N[0]*clhs10*clhs44*clhs46*clhs47*clhs53 - clhs102*clhs452 - clhs128*clhs540 + clhs24*clhs53 + clhs29*clhs536 + clhs42 + clhs506*clhs539 + clhs533*clhs534 + clhs537*clhs7;
const double clhs542 =             clhs5 + clhs541;
const double clhs543 =             clhs506*clhs514;
const double clhs544 =             clhs509*clhs511;
const double clhs545 =             clhs433*clhs7*tau2;
const double clhs546 =             clhs102*clhs139*tau2;
const double clhs547 =             (1.0L/2.0L)*DN(0,1)*tau1;
const double clhs548 =             (1.0L/2.0L)*N[0]*clhs102*clhs53*tau2;
const double clhs549 =             N[0]*clhs10*clhs36*clhs40;
const double clhs550 =             clhs374*clhs53*clhs7;
const double clhs551 =             3*clhs465;
const double clhs552 =             clhs468 + clhs471 + clhs474 + clhs477 + clhs478 + clhs551;
const double clhs553 =             clhs46*clhs492;
const double clhs554 =             -N[0]*clhs44*clhs53*clhs62*clhs8*clhs86 - clhs102*clhs457 - clhs128*clhs553 + clhs21*clhs550 + clhs511*clhs552 + clhs53*clhs74 + clhs53*clhs76 + clhs549*clhs7 + clhs84;
const double clhs555 =             clhs554 + clhs70;
const double clhs556 =             DN(0,0)*tau3;
const double clhs557 =             clhs118 + clhs121*gamma + clhs122*gamma - clhs130*gamma + clhs141*gamma + clhs142*gamma - clhs148*gamma;
const double clhs558 =             N[0]*N[1];
const double clhs559 =             clhs4*clhs558;
const double clhs560 =             DN(0,1)*N[1];
const double clhs561 =             clhs560*clhs8;
const double clhs562 =             clhs233*clhs8;
const double clhs563 =             N[1]*clhs21;
const double clhs564 =             DN(0,0)*N[1];
const double clhs565 =             clhs10*clhs564;
const double clhs566 =             clhs31*clhs565;
const double clhs567 =             clhs10*clhs206;
const double clhs568 =             clhs31*clhs567;
const double clhs569 =             N[1]*clhs29;
const double clhs570 =             clhs31*clhs569;
const double clhs571 =             clhs215*clhs46;
const double clhs572 =             clhs571*clhs8;
const double clhs573 =             clhs3*clhs572;
const double clhs574 =             clhs215*clhs62;
const double clhs575 =             clhs32*clhs574;
const double clhs576 =             clhs3*clhs575;
const double clhs577 =             (1.0L/2.0L)*clhs405*clhs7*tau2;
const double clhs578 =             clhs10*clhs560;
const double clhs579 =             clhs10*clhs233;
const double clhs580 =             N[1]*clhs108;
const double clhs581 =             clhs564*clhs8;
const double clhs582 =             clhs206*clhs8;
const double clhs583 =             N[1]*clhs105;
const double clhs584 =             clhs53*clhs583;
const double clhs585 =             clhs217*clhs3;
const double clhs586 =             clhs216*clhs3;
const double clhs587 =             clhs53*clhs581 + clhs53*clhs582 - clhs53*clhs586 - clhs578 - clhs579 - clhs580 + clhs584 + clhs585;
const double clhs588 =             (1.0L/2.0L)*clhs428*clhs7*tau2;
const double clhs589 =             clhs559 - clhs577*(-clhs561 - clhs562 - clhs563 + clhs566 + clhs568 + clhs570 + clhs573 - clhs576) - clhs587*clhs588;
const double clhs590 =             2*N[1]*clhs102;
const double clhs591 =             N[1]*clhs354 + clhs208 + clhs210 - clhs344*clhs590;
const double clhs592 =             -clhs161;
const double clhs593 =             -clhs221 + clhs592;
const double clhs594 =             clhs10*clhs574;
const double clhs595 =             2*N[1]*clhs8;
const double clhs596 =             clhs102*clhs46;
const double clhs597 =             clhs595*clhs596;
const double clhs598 =             clhs166 + clhs185 + clhs593 - 4*clhs594 + clhs597;
const double clhs599 =             clhs18*clhs212;
const double clhs600 =             clhs209*clhs366;
const double clhs601 =             -clhs158 - clhs163 + clhs167 + clhs176 - clhs179*clhs347 + clhs217*clhs47 - clhs599 + clhs600;
const double clhs602 =             (1.0L/2.0L)*N[1]*bdf0*tau1;
const double clhs603 =             clhs372*clhs44*tau1;
const double clhs604 =             N[0]*clhs17*clhs8;
const double clhs605 =             N[0]*N[1]*clhs102;
const double clhs606 =             clhs10*clhs374*clhs46;
const double clhs607 =             clhs605*clhs62;
const double clhs608 =             N[0]*clhs158 + N[1]*clhs12 + N[1]*clhs23 - N[1]*clhs33 - clhs137*clhs595 + clhs154*clhs604 + clhs379*clhs607 - clhs605*clhs606;
const double clhs609 =             -clhs155;
const double clhs610 =             clhs154*clhs436;
const double clhs611 =             -clhs159 - clhs160 - clhs164 + clhs168 + clhs171 + clhs174 + clhs177;
const double clhs612 =             clhs609 + clhs610 + clhs611;
const double clhs613 =             -clhs183;
const double clhs614 =             clhs154*clhs442;
const double clhs615 =             -clhs187 - clhs189 - clhs190 + clhs192 + clhs195 + clhs197 + clhs200;
const double clhs616 =             clhs613 + clhs614 + clhs615;
const double clhs617 =             clhs154*clhs447;
const double clhs618 =             -clhs617;
const double clhs619 =             N[1]*clhs391;
const double clhs620 =             clhs619*gamma;
const double clhs621 =             2*clhs620;
const double clhs622 =             clhs10*clhs621;
const double clhs623 =             clhs622*clhs7;
const double clhs624 =             N[1]*clhs417;
const double clhs625 =             clhs624*gamma;
const double clhs626 =             2*clhs625;
const double clhs627 =             clhs626*clhs8;
const double clhs628 =             clhs627*clhs7;
const double clhs629 =             N[1]*clhs40*clhs47*clhs53;
const double clhs630 =             clhs460*clhs629;
const double clhs631 =             N[1]*clhs17*clhs47*clhs53;
const double clhs632 =             clhs460*clhs631;
const double clhs633 =             clhs224*clhs483;
const double clhs634 =             clhs633*clhs7;
const double clhs635 =             clhs221*clhs483;
const double clhs636 =             clhs635*clhs7;
const double clhs637 =             clhs165*clhs489;
const double clhs638 =             clhs637*clhs7;
const double clhs639 =             clhs161*clhs492;
const double clhs640 =             clhs639*clhs7;
const double clhs641 =             2*N[1]*clhs10;
const double clhs642 =             clhs44*clhs498*clhs62;
const double clhs643 =             clhs641*clhs642;
const double clhs644 =             clhs44*clhs46*clhs498;
const double clhs645 =             clhs595*clhs644;
const double clhs646 =             -clhs623 - clhs628 + clhs630 + clhs632 + clhs634 + clhs636 + clhs638 + clhs640 + clhs643 + clhs645;
const double clhs647 =             clhs618 + clhs646;
const double clhs648 =             clhs372*clhs7*tau1;
const double clhs649 =             clhs561 + clhs562 + clhs563 - clhs573;
const double clhs650 =             clhs46*clhs605;
const double clhs651 =             clhs233 + clhs560 - clhs650;
const double clhs652 =             (1.0L/2.0L)*clhs102*clhs527*tau2;
const double clhs653 =             clhs651*clhs652;
const double clhs654 =             clhs206 + clhs564 - clhs607;
const double clhs655 =             (1.0L/2.0L)*clhs102*clhs531*tau2;
const double clhs656 =             clhs654*clhs655;
const double clhs657 =             clhs153 - clhs31*clhs656 + clhs648*(-clhs566 - clhs568 - clhs570 + clhs576 + clhs649) + clhs653;
const double clhs658 =             -clhs574;
const double clhs659 =             DN(1,0) + clhs658;
const double clhs660 =             clhs352*clhs659*mu;
const double clhs661 =             clhs506*clhs660;
const double clhs662 =             -clhs571;
const double clhs663 =             DN(1,1) + clhs662;
const double clhs664 =             clhs352*clhs663*mu;
const double clhs665 =             clhs511*clhs664;
const double clhs666 =             clhs161 + clhs221 - clhs572;
const double clhs667 =             -clhs227;
const double clhs668 =             -clhs225 + clhs575 + clhs666 + clhs667;
const double clhs669 =             clhs218*clhs7*tau2;
const double clhs670 =             clhs139*clhs669;
const double clhs671 =             (1.0L/2.0L)*DN(1,0)*tau1;
const double clhs672 =             clhs157*clhs53;
const double clhs673 =             clhs36*clhs7*clhs8;
const double clhs674 =             DN(1,0)*clhs102;
const double clhs675 =             -N[1]*clhs10*clhs44*clhs46*clhs47*clhs53 - clhs102*clhs621 + clhs164*clhs53 + clhs165*clhs536 + clhs171 + clhs212*clhs673 - clhs229*clhs540 + clhs534*clhs672 + clhs539*clhs674;
const double clhs676 =             clhs155 + clhs675;
const double clhs677 =             (1.0L/2.0L)*clhs102*clhs405*tau2;
const double clhs678 =             (1.0L/2.0L)*clhs102*clhs428*clhs53*tau2;
const double clhs679 =             -clhs587*clhs648 - clhs651*clhs677 + clhs654*clhs678;
const double clhs680 =             clhs506*clhs664;
const double clhs681 =             clhs511*clhs660;
const double clhs682 =             (1.0L/2.0L)*DN(1,1)*tau1;
const double clhs683 =             clhs10*clhs36*clhs7;
const double clhs684 =             DN(1,1)*clhs102;
const double clhs685 =             -N[1]*clhs44*clhs53*clhs62*clhs8*clhs86 - clhs102*clhs626 + clhs161*clhs550 + clhs187*clhs53 + clhs189*clhs53 + clhs195 + clhs209*clhs683 - clhs229*clhs553 + clhs552*clhs684;
const double clhs686 =             clhs183 + clhs685;
const double clhs687 =             DN(1,0)*clhs102*tau2;
const double clhs688 =             DN(1,1)*clhs102*tau2;
const double clhs689 =             clhs219 + clhs222*gamma + clhs223*gamma - clhs230*gamma + clhs238*gamma + clhs239*gamma - clhs244*gamma;
const double clhs690 =             N[0]*N[2];
const double clhs691 =             clhs4*clhs690;
const double clhs692 =             DN(0,1)*N[2];
const double clhs693 =             clhs692*clhs8;
const double clhs694 =             clhs328*clhs8;
const double clhs695 =             N[2]*clhs21;
const double clhs696 =             DN(0,0)*N[2];
const double clhs697 =             clhs10*clhs696;
const double clhs698 =             clhs31*clhs697;
const double clhs699 =             clhs10*clhs301;
const double clhs700 =             clhs31*clhs699;
const double clhs701 =             N[2]*clhs29;
const double clhs702 =             clhs31*clhs701;
const double clhs703 =             clhs310*clhs46;
const double clhs704 =             clhs703*clhs8;
const double clhs705 =             clhs3*clhs704;
const double clhs706 =             clhs310*clhs62;
const double clhs707 =             clhs32*clhs706;
const double clhs708 =             clhs3*clhs707;
const double clhs709 =             clhs10*clhs692;
const double clhs710 =             clhs10*clhs328;
const double clhs711 =             N[2]*clhs108;
const double clhs712 =             clhs696*clhs8;
const double clhs713 =             clhs301*clhs8;
const double clhs714 =             N[2]*clhs105;
const double clhs715 =             clhs53*clhs714;
const double clhs716 =             clhs3*clhs312;
const double clhs717 =             clhs3*clhs311;
const double clhs718 =             clhs53*clhs712 + clhs53*clhs713 - clhs53*clhs717 - clhs709 - clhs710 - clhs711 + clhs715 + clhs716;
const double clhs719 =             -clhs577*(-clhs693 - clhs694 - clhs695 + clhs698 + clhs700 + clhs702 + clhs705 - clhs708) - clhs588*clhs718 + clhs691;
const double clhs720 =             2*N[2]*clhs102;
const double clhs721 =             N[2]*clhs354 + clhs303 + clhs305 - clhs344*clhs720;
const double clhs722 =             -clhs256;
const double clhs723 =             -clhs316 + clhs722;
const double clhs724 =             clhs10*clhs706;
const double clhs725 =             2*N[2]*clhs8;
const double clhs726 =             clhs596*clhs725;
const double clhs727 =             clhs261 + clhs280 + clhs723 - 4*clhs724 + clhs726;
const double clhs728 =             clhs18*clhs307;
const double clhs729 =             clhs304*clhs366;
const double clhs730 =             -clhs253 - clhs258 + clhs262 + clhs271 - clhs274*clhs347 + clhs312*clhs47 - clhs728 + clhs729;
const double clhs731 =             (1.0L/2.0L)*N[2]*bdf0*tau1;
const double clhs732 =             N[0]*N[2]*clhs102;
const double clhs733 =             clhs62*clhs732;
const double clhs734 =             N[0]*clhs253 + N[2]*clhs12 + N[2]*clhs23 - N[2]*clhs33 - clhs137*clhs725 + clhs249*clhs604 + clhs379*clhs733 - clhs606*clhs732;
const double clhs735 =             -clhs250;
const double clhs736 =             clhs249*clhs436;
const double clhs737 =             -clhs254 - clhs255 - clhs259 + clhs263 + clhs266 + clhs269 + clhs272;
const double clhs738 =             clhs735 + clhs736 + clhs737;
const double clhs739 =             -clhs278;
const double clhs740 =             clhs249*clhs442;
const double clhs741 =             -clhs282 - clhs284 - clhs285 + clhs287 + clhs290 + clhs292 + clhs295;
const double clhs742 =             clhs739 + clhs740 + clhs741;
const double clhs743 =             clhs249*clhs447;
const double clhs744 =             -clhs743;
const double clhs745 =             N[2]*clhs391;
const double clhs746 =             clhs745*gamma;
const double clhs747 =             2*clhs746;
const double clhs748 =             clhs10*clhs747;
const double clhs749 =             clhs7*clhs748;
const double clhs750 =             N[2]*clhs417;
const double clhs751 =             clhs750*gamma;
const double clhs752 =             2*clhs751;
const double clhs753 =             clhs752*clhs8;
const double clhs754 =             clhs7*clhs753;
const double clhs755 =             N[2]*clhs40*clhs47*clhs53;
const double clhs756 =             clhs460*clhs755;
const double clhs757 =             N[2]*clhs17*clhs47*clhs53;
const double clhs758 =             clhs460*clhs757;
const double clhs759 =             clhs319*clhs483;
const double clhs760 =             clhs7*clhs759;
const double clhs761 =             clhs316*clhs483;
const double clhs762 =             clhs7*clhs761;
const double clhs763 =             clhs260*clhs489;
const double clhs764 =             clhs7*clhs763;
const double clhs765 =             clhs256*clhs492;
const double clhs766 =             clhs7*clhs765;
const double clhs767 =             2*N[2]*clhs10;
const double clhs768 =             clhs642*clhs767;
const double clhs769 =             clhs644*clhs725;
const double clhs770 =             -clhs749 - clhs754 + clhs756 + clhs758 + clhs760 + clhs762 + clhs764 + clhs766 + clhs768 + clhs769;
const double clhs771 =             clhs744 + clhs770;
const double clhs772 =             clhs693 + clhs694 + clhs695 - clhs705;
const double clhs773 =             clhs46*clhs732;
const double clhs774 =             clhs328 + clhs692 - clhs773;
const double clhs775 =             clhs652*clhs774;
const double clhs776 =             clhs301 + clhs696 - clhs733;
const double clhs777 =             clhs655*clhs776;
const double clhs778 =             clhs248 - clhs31*clhs777 + clhs648*(-clhs698 - clhs700 - clhs702 + clhs708 + clhs772) + clhs775;
const double clhs779 =             -clhs706;
const double clhs780 =             DN(2,0) + clhs779;
const double clhs781 =             clhs352*clhs780*mu;
const double clhs782 =             clhs506*clhs781;
const double clhs783 =             -clhs703;
const double clhs784 =             DN(2,1) + clhs783;
const double clhs785 =             clhs352*clhs784*mu;
const double clhs786 =             clhs511*clhs785;
const double clhs787 =             clhs256 + clhs316 - clhs704;
const double clhs788 =             -clhs322;
const double clhs789 =             -clhs320 + clhs707 + clhs787 + clhs788;
const double clhs790 =             clhs313*clhs7*tau2;
const double clhs791 =             clhs139*clhs790;
const double clhs792 =             (1.0L/2.0L)*DN(2,0)*tau1;
const double clhs793 =             clhs252*clhs53;
const double clhs794 =             DN(2,0)*clhs102;
const double clhs795 =             -N[2]*clhs10*clhs44*clhs46*clhs47*clhs53 - clhs102*clhs747 + clhs259*clhs53 + clhs260*clhs536 + clhs266 + clhs307*clhs673 - clhs324*clhs540 + clhs534*clhs793 + clhs539*clhs794;
const double clhs796 =             clhs250 + clhs795;
const double clhs797 =             -clhs648*clhs718 - clhs677*clhs774 + clhs678*clhs776;
const double clhs798 =             clhs506*clhs785;
const double clhs799 =             clhs511*clhs781;
const double clhs800 =             (1.0L/2.0L)*DN(2,1)*tau1;
const double clhs801 =             DN(2,1)*clhs102;
const double clhs802 =             -N[2]*clhs44*clhs53*clhs62*clhs8*clhs86 - clhs102*clhs752 + clhs256*clhs550 + clhs282*clhs53 + clhs284*clhs53 + clhs290 + clhs304*clhs683 - clhs324*clhs553 + clhs552*clhs801;
const double clhs803 =             clhs278 + clhs802;
const double clhs804 =             DN(2,0)*clhs102*tau2;
const double clhs805 =             DN(2,1)*clhs102*tau2;
const double clhs806 =             clhs314 + clhs317*gamma + clhs318*gamma - clhs325*gamma + clhs333*gamma + clhs334*gamma - clhs339*gamma;
const double clhs807 =             clhs0*clhs69;
const double clhs808 =             clhs10*clhs347;
const double clhs809 =             U(0,2)*clhs9 + U(1,2)*clhs156 + U(2,2)*clhs251 - clhs18*clhs596 - clhs25 - clhs26 - clhs27 + clhs808;
const double clhs810 =             (2.0L/3.0L)*clhs102*clhs809*v_sc;
const double clhs811 =             DN(0,0)*clhs352*clhs7*mu;
const double clhs812 =             (2.0L/3.0L)*DN(0,1)*clhs352*clhs7*mu;
const double clhs813 =             clhs3*clhs808;
const double clhs814 =             -clhs813;
const double clhs815 =             clhs123 + clhs29 + clhs814;
const double clhs816 =             -clhs22 + clhs381 + clhs430*clhs47 + clhs815;
const double clhs817 =             clhs105*clhs77;
const double clhs818 =             clhs10*clhs36;
const double clhs819 =             clhs108*clhs818;
const double clhs820 =             clhs113*clhs86 - clhs596*clhs96 - clhs73 - clhs75 + clhs80 - clhs817 + clhs819 + clhs92;
const double clhs821 =             clhs444 + clhs70 - clhs97;
const double clhs822 =             clhs377 + clhs378 - 6*clhs55;
const double clhs823 =             clhs145 - clhs29;
const double clhs824 =             clhs31*clhs380;
const double clhs825 =             clhs102*clhs31*clhs46*clhs8;
const double clhs826 =             clhs3*clhs825;
const double clhs827 =             -clhs409;
const double clhs828 =             clhs105 + clhs407 + clhs827;
const double clhs829 =             clhs110 + clhs408*clhs53 - clhs533 + clhs828;
const double clhs830 =             clhs123 + clhs29 - clhs361;
const double clhs831 =             -clhs145;
const double clhs832 =             clhs430*clhs79;
const double clhs833 =             -clhs143 + clhs830 + clhs831 + clhs832;
const double clhs834 =             (1.0L/2.0L)*clhs102*clhs833*tau2;
const double clhs835 =             (1.0L/2.0L)*clhs102*clhs116*tau2;
const double clhs836 =             (1.0L/2.0L)*DN(0,1)*clhs53*tau3;
const double clhs837 =             clhs7*clhs833*tau2;
const double clhs838 =             clhs102*clhs116*tau2;
const double clhs839 =             clhs118 + clhs121 + clhs122 - clhs125 - clhs127 - clhs130 + clhs132;
const double clhs840 =             clhs29 + clhs72 + clhs814;
const double clhs841 =             -clhs123 + clhs143 + clhs361 + clhs823 - clhs832;
const double clhs842 =             clhs102*clhs841*tau2;
const double clhs843 =             clhs119 - clhs141 - clhs142 + clhs144 + clhs146 + clhs148 - clhs150;
const double clhs844 =             clhs440 + clhs98;
const double clhs845 =             DN(0,1)*tau3;
const double clhs846 =             clhs558*clhs69;
const double clhs847 =             -clhs3*clhs594 + clhs565 + clhs567 + clhs569;
const double clhs848 =             clhs571*clhs79;
const double clhs849 =             clhs3*clhs848 - clhs31*clhs561 - clhs31*clhs562 - clhs31*clhs563 + clhs847;
const double clhs850 =             clhs581 + clhs582 + clhs583 - clhs586;
const double clhs851 =             clhs53*clhs578;
const double clhs852 =             clhs53*clhs579;
const double clhs853 =             clhs53*clhs580;
const double clhs854 =             clhs53*clhs585 + clhs850 - clhs851 - clhs852 - clhs853;
const double clhs855 =             clhs577*clhs854 + clhs588*clhs849 + clhs846;
const double clhs856 =             clhs347*clhs641;
const double clhs857 =             -clhs856;
const double clhs858 =             clhs165 + clhs224 + clhs857;
const double clhs859 =             clhs156*clhs8;
const double clhs860 =             -clhs859;
const double clhs861 =             -clhs162 + clhs47*clhs571 + clhs858 + clhs860;
const double clhs862 =             clhs209*clhs77;
const double clhs863 =             clhs212*clhs818;
const double clhs864 =             -clhs186 - clhs188 + clhs191 + clhs199 - clhs202*clhs596 + clhs216*clhs86 - clhs862 + clhs863;
const double clhs865 =             N[0]*clhs10*clhs40;
const double clhs866 =             clhs535*clhs62*clhs8;
const double clhs867 =             N[0]*clhs186 + N[1]*clhs73 + N[1]*clhs75 - N[1]*clhs80 - clhs109*clhs641 + clhs154*clhs865 - clhs605*clhs866 + clhs650*clhs822;
const double clhs868 =             (1.0L/2.0L)*clhs102*clhs428*tau2;
const double clhs869 =             (1.0L/2.0L)*clhs102*clhs405*clhs53*tau2;
const double clhs870 =             clhs648*clhs854 + clhs651*clhs869 - clhs654*clhs868;
const double clhs871 =             clhs219 + clhs222 + clhs223 - clhs226 - clhs228 - clhs230 + clhs231;
const double clhs872 =             clhs153 - clhs31*clhs653 + clhs648*clhs849 + clhs656;
const double clhs873 =             clhs165 + clhs224 - clhs594;
const double clhs874 =             -clhs242;
const double clhs875 =             -clhs240 + clhs848 + clhs873 + clhs874;
const double clhs876 =             clhs237*clhs504;
const double clhs877 =             clhs220 - clhs238 - clhs239 + clhs241 + clhs243 + clhs244 - clhs245;
const double clhs878 =             clhs69*clhs690;
const double clhs879 =             -clhs3*clhs724 + clhs697 + clhs699 + clhs701;
const double clhs880 =             clhs703*clhs79;
const double clhs881 =             clhs3*clhs880 - clhs31*clhs693 - clhs31*clhs694 - clhs31*clhs695 + clhs879;
const double clhs882 =             clhs712 + clhs713 + clhs714 - clhs717;
const double clhs883 =             clhs53*clhs709;
const double clhs884 =             clhs53*clhs710;
const double clhs885 =             clhs53*clhs711;
const double clhs886 =             clhs53*clhs716 + clhs882 - clhs883 - clhs884 - clhs885;
const double clhs887 =             clhs577*clhs886 + clhs588*clhs881 + clhs878;
const double clhs888 =             clhs347*clhs767;
const double clhs889 =             -clhs888;
const double clhs890 =             clhs260 + clhs319 + clhs889;
const double clhs891 =             clhs251*clhs8;
const double clhs892 =             -clhs891;
const double clhs893 =             -clhs257 + clhs47*clhs703 + clhs890 + clhs892;
const double clhs894 =             clhs304*clhs77;
const double clhs895 =             clhs307*clhs818;
const double clhs896 =             -clhs281 - clhs283 + clhs286 + clhs294 - clhs297*clhs596 + clhs311*clhs86 - clhs894 + clhs895;
const double clhs897 =             N[0]*clhs281 + N[2]*clhs73 + N[2]*clhs75 - N[2]*clhs80 - clhs109*clhs767 + clhs249*clhs865 - clhs732*clhs866 + clhs773*clhs822;
const double clhs898 =             clhs648*clhs886 + clhs774*clhs869 - clhs776*clhs868;
const double clhs899 =             clhs314 + clhs317 + clhs318 - clhs321 - clhs323 - clhs325 + clhs326;
const double clhs900 =             clhs248 - clhs31*clhs775 + clhs648*clhs881 + clhs777;
const double clhs901 =             clhs260 + clhs319 - clhs724;
const double clhs902 =             -clhs337;
const double clhs903 =             -clhs335 + clhs880 + clhs901 + clhs902;
const double clhs904 =             clhs332*clhs504;
const double clhs905 =             clhs315 - clhs333 - clhs334 + clhs336 + clhs338 + clhs339 - clhs340;
const double clhs906 =             (1.0L/2.0L)*N[0]*clhs7*tau2;
const double clhs907 =             clhs102*clhs47;
const double clhs908 =             clhs107*clhs907;
const double clhs909 =             clhs53*clhs535;
const double clhs910 =             clhs142*clhs909;
const double clhs911 =             clhs102*clhs41;
const double clhs912 =             clhs102*clhs537;
const double clhs913 =             clhs23*clhs479;
const double clhs914 =             clhs10*clhs374*clhs46*clhs53;
const double clhs915 =             clhs128*clhs914;
const double clhs916 =             DN(0,0)*clhs489;
const double clhs917 =             -7*clhs478 + clhs497;
const double clhs918 =             clhs360*clhs917;
const double clhs919 =             clhs374*clhs53;
const double clhs920 =             -clhs102*clhs549 - clhs102*clhs83 - clhs122*clhs919 - clhs479*clhs75;
const double clhs921 =             clhs53*clhs535*clhs62*clhs8;
const double clhs922 =             -7*clhs465 + clhs467 + clhs470 + clhs473 + clhs476 + clhs495 + clhs496;
const double clhs923 =             -DN(0,0)*clhs10*clhs47*clhs479 - DN(0,1)*clhs492 + clhs128*clhs921 - clhs430*clhs922 + clhs456 + clhs920;
const double clhs924 =             (1.0L/4.0L)*tau2*(clhs13 + clhs19 + clhs24 + clhs35 + clhs43 - clhs437 + clhs5 + clhs50 + clhs61);
const double clhs925 =             clhs435 + clhs541;
const double clhs926 =             (1.0L/4.0L)*tau2*(-clhs443 + clhs70 + clhs74 + clhs76 + clhs78 + clhs82 + clhs85 + clhs89 + clhs94);
const double clhs927 =             clhs440 + clhs554;
const double clhs928 =             N[0]*clhs7*(clhs521 + clhs840);
const double clhs929 =             clhs3*(U(0,3)*bdf0 + Un(0,3)*bdf1 + Unn(0,3)*bdf2);
const double clhs930 =             clhs154*(U(1,3)*bdf0 + Un(1,3)*bdf1 + Unn(1,3)*bdf2);
const double clhs931 =             clhs249*(U(2,3)*bdf0 + Un(2,3)*bdf1 + Unn(2,3)*bdf2);
const double clhs932 =             clhs393*clhs447;
const double clhs933 =             clhs4*clhs77;
const double clhs934 =             clhs18*clhs69;
const double clhs935 =             clhs391*clhs397*gamma;
const double clhs936 =             clhs395*clhs417*gamma;
const double clhs937 =             clhs10*clhs36*clhs7*clhs8;
const double clhs938 =             clhs40*clhs937;
const double clhs939 =             clhs17*clhs937;
const double clhs940 =             clhs102*clhs28*clhs539;
const double clhs941 =             clhs102*clhs20*clhs552;
const double clhs942 =             clhs483*clhs7;
const double clhs943 =             clhs147*clhs942;
const double clhs944 =             clhs129*clhs942;
const double clhs945 =             -clhs929 - clhs930 - clhs931 + clhs932 + clhs933 + clhs934 - clhs935 - clhs936 + clhs938 + clhs939 + clhs940 + clhs941 - clhs943 - clhs944;
const double clhs946 =             (1.0L/2.0L)*clhs945*gamma*tau3;
const double clhs947 =             -clhs453 - clhs458;
const double clhs948 =             clhs10*clhs102;
const double clhs949 =             clhs362*clhs498 + clhs461*clhs948 + clhs463*clhs948 + clhs484 + clhs486 + clhs490 + clhs493 + clhs498*clhs813 + clhs947;
const double clhs950 =             clhs10*clhs102*clhs374;
const double clhs951 =             -6*N[0]*U(0,3) - 6*N[1]*U(1,3) - 6*N[2]*U(2,3) + 9*clhs102*clhs480*clhs53 - 6*clhs475*clhs53 + clhs538 + clhs551;
const double clhs952 =             clhs454 + clhs459 - clhs462 - clhs464 - clhs485 - clhs487 - clhs491 - clhs494 - clhs500 - clhs501;
const double clhs953 =             clhs449 + clhs952;
const double clhs954 =             clhs516 + clhs830;
const double clhs955 =             (1.0L/2.0L)*clhs102*clhs954*gamma*tau3;
const double clhs956 =             clhs448 + clhs952;
const double clhs957 =             (1.0L/3.0L)*DN(0,0)*clhs102;
const double clhs958 =             3*N[0]*clhs102*clhs345*v_sc;
const double clhs959 =             3*N[0]*clhs345*clhs352*clhs7*mu;
const double clhs960 =             clhs350*clhs352*mu;
const double clhs961 =             clhs3*clhs960;
const double clhs962 =             3*clhs352*clhs7*clhs8*mu;
const double clhs963 =             clhs352*clhs7*clhs77*mu;
const double clhs964 =             3*N[0]*k_sc;
const double clhs965 =             clhs347*clhs475;
const double clhs966 =             clhs28*clhs948 + clhs348*clhs40 - clhs388 - clhs389 - clhs390 - clhs403*clhs51 - clhs403*clhs55 + clhs965;
const double clhs967 =             1.0/c_v;
const double clhs968 =             c_v*clhs6*k_sc/lambda + 1;
const double clhs969 =             3*clhs102*clhs967*clhs968*lambda;
const double clhs970 =             3*N[0]*clhs62*clhs7;
const double clhs971 =             -2*N[0]*clhs10*clhs102*clhs350*v_sc + clhs10*clhs7*clhs961 - clhs356*clhs962 - clhs363*clhs963 - clhs8*clhs958 + clhs8*clhs959 - clhs964*clhs966 + clhs969*(-DN(0,0)*clhs475 + clhs105*clhs395 + clhs3*clhs965 + clhs30*clhs948 - clhs450 + clhs506*clhs51 + clhs506*clhs55 - clhs51*clhs970 - clhs55*clhs970);
const double clhs972 =             (1.0L/3.0L)*DN(0,1)*clhs102;
const double clhs973 =             clhs352*clhs809*mu;
const double clhs974 =             clhs3*clhs973;
const double clhs975 =             3*clhs10*clhs352*clhs7*mu;
const double clhs976 =             clhs18*clhs352*clhs7*mu;
const double clhs977 =             clhs475*clhs596;
const double clhs978 =             clhs17*clhs948 + clhs20*clhs348 - clhs414 - clhs415 - clhs416 - clhs426*clhs51 - clhs426*clhs55 + clhs977;
const double clhs979 =             3*N[0]*clhs46*clhs7;
const double clhs980 =             2*N[0]*clhs102*clhs8*clhs809*v_sc - clhs10*clhs958 + clhs10*clhs959 - clhs356*clhs975 - clhs534*clhs974 + clhs816*clhs976 - clhs964*clhs978 + clhs969*(-DN(0,1)*clhs475 + clhs108*clhs397 + clhs22*clhs348 + clhs3*clhs977 - clhs455 + clhs51*clhs511 - clhs51*clhs979 + clhs511*clhs55 - clhs55*clhs979);
const double clhs981 =             (1.0L/3.0L)*DN(0,1)*clhs7;
const double clhs982 =             3*clhs352*clhs513*mu;
const double clhs983 =             3*clhs967*clhs968*lambda;
const double clhs984 =             -clhs408;
const double clhs985 =             3*clhs345*clhs352*mu;
const double clhs986 =             N[0]*clhs985;
const double clhs987 =             -clhs10*clhs982 + clhs18*clhs509 + clhs983*(clhs106 + clhs108 + clhs984) + clhs986;
const double clhs988 =             (1.0L/3.0L)*DN(0,0)*clhs7;
const double clhs989 =             -clhs509*clhs86 - clhs8*clhs982 + clhs815*clhs983 + clhs961;
const double clhs990 =             (1.0L/2.0L)*N[0]*clhs7*(clhs108 + clhs11 + clhs828 + clhs984);
const double clhs991 =             clhs428*clhs53*tau2;
const double clhs992 =             3*clhs29;
const double clhs993 =             clhs405*clhs53*tau2;
const double clhs994 =             (1.0L/2.0L)*N[0]*clhs102;
const double clhs995 =             clhs910 + clhs911 + clhs912 + clhs913;
const double clhs996 =             N[0]*clhs53*clhs7;
const double clhs997 =             DN(0,0)*clhs539 + clhs348*clhs533 - clhs360*clhs489 - clhs452 - clhs48*clhs996 + clhs995;
const double clhs998 =             (1.0L/2.0L)*clhs839*tau2;
const double clhs999 =             (1.0L/2.0L)*N[0]*clhs102*gamma*tau3;
const double clhs1000 =             clhs929 + clhs930 + clhs931 - clhs932 - clhs933 - clhs934 + clhs935 + clhs936 - clhs938 - clhs939 - clhs940 - clhs941 + clhs943 + clhs944;
const double clhs1001 =             3*clhs352*clhs508*mu;
const double clhs1002 =             clhs1001*clhs8 - clhs514*clhs77 - clhs983*(clhs104 + clhs105 + clhs827) - clhs986;
const double clhs1003 =             clhs10*clhs1001 + clhs47*clhs514 + clhs974 - clhs983*(clhs120 + clhs21 + clhs520);
const double clhs1004 =             clhs53*clhs531*tau2;
const double clhs1005 =             3*clhs21;
const double clhs1006 =             clhs527*clhs53*tau2;
const double clhs1007 =             -DN(0,1)*clhs552 + clhs430*clhs492 + clhs457 - clhs479*clhs73 + clhs87*clhs996 + clhs920;
const double clhs1008 =             (1.0L/2.0L)*clhs843*tau2;
const double clhs1009 =             DN(0,0)*clhs102*clhs967*clhs968*lambda;
const double clhs1010 =             DN(0,1)*clhs102*clhs967*clhs968*lambda;
const double clhs1011 =             N[0]*clhs102*gamma;
const double clhs1012 =             clhs372*gamma*tau1;
const double clhs1013 =             clhs102*clhs954*gamma*tau3;
const double clhs1014 =             (1.0L/2.0L)*N[0]*clhs102*gamma*tau2;
const double clhs1015 =             (1.0L/2.0L)*DN(0,0)*clhs53*tau2;
const double clhs1016 =             (1.0L/2.0L)*DN(0,1)*clhs53*tau2;
const double clhs1017 =             clhs649 + clhs847;
const double clhs1018 =             (1.0L/2.0L)*clhs7*clhs945*gamma*tau3;
const double clhs1019 =             (1.0L/4.0L)*clhs531*clhs7*tau2;
const double clhs1020 =             clhs154*clhs451;
const double clhs1021 =             clhs851*clhs907;
const double clhs1022 =             clhs852*clhs907;
const double clhs1023 =             12*clhs10*clhs102*clhs53;
const double clhs1024 =             clhs1023*clhs569;
const double clhs1025 =             clhs584*clhs907;
const double clhs1026 =             clhs853*clhs907;
const double clhs1027 =             4*clhs10*clhs102*clhs53;
const double clhs1028 =             clhs1027*clhs563;
const double clhs1029 =             12*N[0]*N[1]*clhs53;
const double clhs1030 =             clhs10*clhs46*clhs7*clhs8;
const double clhs1031 =             clhs1029*clhs1030;
const double clhs1032 =             clhs489*clhs564;
const double clhs1033 =             clhs206*clhs489;
const double clhs1034 =             clhs574*clhs917;
const double clhs1035 =             clhs1034*clhs3;
const double clhs1036 =             clhs1020 - clhs1021 - clhs1022 - clhs1024 - clhs1025 - clhs1026 - clhs1028 + clhs1031 - clhs1032 - clhs1033 - clhs1035;
const double clhs1037 =             (1.0L/4.0L)*clhs527*clhs7*tau2;
const double clhs1038 =             clhs154*clhs456;
const double clhs1039 =             clhs102*clhs47*clhs53;
const double clhs1040 =             4*clhs10*clhs102;
const double clhs1041 =             12*clhs102*clhs53*clhs8;
const double clhs1042 =             clhs10*clhs62*clhs7*clhs8;
const double clhs1043 =             clhs571*clhs922;
const double clhs1044 =             clhs1029*clhs1042 + clhs1038 - clhs1039*clhs565 - clhs1039*clhs567 - clhs1039*clhs569 - clhs1040*clhs584 - clhs1040*clhs853 - clhs1041*clhs563 - clhs1043*clhs3 - clhs233*clhs492 - clhs492*clhs560;
const double clhs1045 =             clhs10*clhs951;
const double clhs1046 =             clhs8*clhs951;
const double clhs1047 =             -clhs1017*clhs1018 - clhs1019*clhs1036 - clhs1037*clhs1044 + clhs447*clhs558 + clhs603*(-clhs10*clhs1020 - clhs1038*clhs8 - clhs1045*clhs607 - clhs1046*clhs650 - clhs498*clhs561 - clhs498*clhs562 - clhs498*clhs565 - clhs498*clhs567 - clhs563*clhs922 - clhs569*clhs917 + clhs584*clhs950 + clhs853*clhs950);
const double clhs1048 =             (1.0L/4.0L)*tau2*(clhs155 + clhs159 + clhs160 + clhs164 + clhs169 + clhs172 + clhs175 + clhs178 - clhs610);
const double clhs1049 =             (1.0L/4.0L)*tau2*(clhs183 + clhs187 + clhs189 + clhs190 + clhs193 + clhs196 + clhs198 + clhs201 - clhs614);
const double clhs1050 =             -clhs622 - clhs627;
const double clhs1051 =             clhs1050 + clhs498*clhs597 + clhs498*clhs856 + clhs629*clhs948 + clhs631*clhs948 + clhs633 + clhs635 + clhs637 + clhs639;
const double clhs1052 =             clhs623 + clhs628 - clhs630 - clhs632 - clhs634 - clhs636 - clhs638 - clhs640 - clhs643 - clhs645;
const double clhs1053 =             clhs1052 + clhs617;
const double clhs1054 =             3*N[1]*clhs102*clhs345*v_sc;
const double clhs1055 =             clhs102*clhs350*v_sc;
const double clhs1056 =             3*N[1]*clhs345*clhs352*clhs7*mu;
const double clhs1057 =             clhs350*clhs352*clhs7*mu;
const double clhs1058 =             3*N[1]*k_sc;
const double clhs1059 =             3*N[1]*clhs62*clhs7;
const double clhs1060 =             -clhs1054*clhs8 - clhs1055*clhs641 + clhs1056*clhs8 + clhs1057*clhs641 - clhs1058*clhs966 - clhs591*clhs962 - clhs598*clhs963 + clhs969*(-DN(1,0)*clhs475 - clhs1059*clhs51 - clhs1059*clhs55 + clhs154*clhs965 + clhs166*clhs948 + clhs209*clhs395 + clhs51*clhs674 + clhs55*clhs674 - clhs619);
const double clhs1061 =             clhs102*clhs809*v_sc;
const double clhs1062 =             clhs352*clhs7*clhs809*mu;
const double clhs1063 =             3*N[1]*clhs46*clhs7;
const double clhs1064 =             -clhs10*clhs1054 + clhs10*clhs1056 - clhs1058*clhs978 + clhs1061*clhs595 - clhs1062*clhs595 - clhs591*clhs975 + clhs861*clhs976 + clhs969*(-DN(1,1)*clhs475 - clhs1063*clhs51 - clhs1063*clhs55 + clhs154*clhs977 + clhs162*clhs348 + clhs212*clhs397 + clhs51*clhs684 + clhs55*clhs684 - clhs624);
const double clhs1065 =             clhs18*clhs660;
const double clhs1066 =             3*clhs352*clhs663*mu;
const double clhs1067 =             clhs10*clhs1066;
const double clhs1068 =             clhs114*clhs590;
const double clhs1069 =             -clhs1068;
const double clhs1070 =             clhs983*(clhs1069 + clhs210 + clhs212);
const double clhs1071 =             N[1]*clhs985;
const double clhs1072 =             -clhs1071;
const double clhs1073 =             clhs660*clhs86;
const double clhs1074 =             clhs1066*clhs8;
const double clhs1075 =             clhs858*clhs983;
const double clhs1076 =             clhs154*clhs960;
const double clhs1077 =             clhs578 + clhs579 + clhs580 - clhs585 + clhs850;
const double clhs1078 =             (1.0L/2.0L)*clhs1077*clhs7;
const double clhs1079 =             clhs239*clhs909;
const double clhs1080 =             clhs102*clhs170;
const double clhs1081 =             clhs212*clhs400;
const double clhs1082 =             clhs163*clhs479;
const double clhs1083 =             -clhs1079 - clhs1080 - clhs1081 - clhs1082;
const double clhs1084 =             clhs348*clhs672;
const double clhs1085 =             N[1]*clhs53*clhs7;
const double clhs1086 =             clhs1085*clhs48;
const double clhs1087 =             DN(1,0)*clhs539;
const double clhs1088 =             clhs489*clhs574;
const double clhs1089 =             N[1]*clhs992 - clhs535*clhs607 + 3*clhs565 + 3*clhs567 + clhs649;
const double clhs1090 =             (1.0L/2.0L)*clhs53*clhs531*clhs7*tau2;
const double clhs1091 =             (1.0L/2.0L)*clhs102*clhs218*tau2;
const double clhs1092 =             (1.0L/2.0L)*clhs925*tau2;
const double clhs1093 =             (1.0L/2.0L)*clhs372*clhs7*tau1;
const double clhs1094 =             (1.0L/2.0L)*clhs102*clhs654*gamma*tau3;
const double clhs1095 =             (1.0L/2.0L)*clhs527*clhs53*clhs7*tau2;
const double clhs1096 =             (1.0L/2.0L)*clhs102*clhs945*gamma*tau3;
const double clhs1097 =             -clhs1004*clhs1078 + clhs1044*clhs1093 - clhs1095*(N[1]*clhs1005 - clhs374*clhs650 + 3*clhs561 + 3*clhs562 + clhs847) + clhs1096*clhs651 + clhs846;
const double clhs1098 =             3*clhs352*clhs659*mu;
const double clhs1099 =             clhs112*clhs590;
const double clhs1100 =             -clhs1099;
const double clhs1101 =             clhs1072 + clhs1098*clhs8 - clhs664*clhs77 - clhs983*(clhs1100 + clhs208 + clhs209);
const double clhs1102 =             -clhs597;
const double clhs1103 =             clhs10*clhs1098 + clhs154*clhs973 + clhs47*clhs664 - clhs983*(clhs1102 + clhs161 + clhs221);
const double clhs1104 =             -clhs102*clhs194 - clhs188*clhs479 - clhs209*clhs423 - clhs223*clhs919;
const double clhs1105 =             -DN(1,1)*clhs552 + clhs1085*clhs87 + clhs1104 - clhs186*clhs479 + clhs492*clhs571 + clhs626;
const double clhs1106 =             (1.0L/2.0L)*clhs102*clhs237*tau2;
const double clhs1107 =             (1.0L/2.0L)*clhs877*tau2;
const double clhs1108 =             clhs372*clhs7*gamma*tau1;
const double clhs1109 =             clhs1017*clhs1108 + clhs153;
const double clhs1110 =             clhs666 + clhs873;
const double clhs1111 =             (1.0L/2.0L)*DN(1,0)*clhs53*tau2;
const double clhs1112 =             (1.0L/2.0L)*DN(1,1)*clhs53*tau2;
const double clhs1113 =             clhs772 + clhs879;
const double clhs1114 =             clhs249*clhs451;
const double clhs1115 =             clhs883*clhs907;
const double clhs1116 =             clhs884*clhs907;
const double clhs1117 =             clhs1023*clhs701;
const double clhs1118 =             clhs715*clhs907;
const double clhs1119 =             clhs885*clhs907;
const double clhs1120 =             clhs1027*clhs695;
const double clhs1121 =             12*N[0]*N[2]*clhs53;
const double clhs1122 =             clhs1030*clhs1121;
const double clhs1123 =             clhs489*clhs696;
const double clhs1124 =             clhs301*clhs489;
const double clhs1125 =             clhs706*clhs917;
const double clhs1126 =             clhs1125*clhs3;
const double clhs1127 =             clhs1114 - clhs1115 - clhs1116 - clhs1117 - clhs1118 - clhs1119 - clhs1120 + clhs1122 - clhs1123 - clhs1124 - clhs1126;
const double clhs1128 =             clhs249*clhs456;
const double clhs1129 =             clhs703*clhs922;
const double clhs1130 =             -clhs1039*clhs697 - clhs1039*clhs699 - clhs1039*clhs701 - clhs1040*clhs715 - clhs1040*clhs885 - clhs1041*clhs695 + clhs1042*clhs1121 + clhs1128 - clhs1129*clhs3 - clhs328*clhs492 - clhs492*clhs692;
const double clhs1131 =             -clhs1018*clhs1113 - clhs1019*clhs1127 - clhs1037*clhs1130 + clhs447*clhs690 + clhs603*(-clhs10*clhs1114 - clhs1045*clhs733 - clhs1046*clhs773 - clhs1128*clhs8 - clhs498*clhs693 - clhs498*clhs694 - clhs498*clhs697 - clhs498*clhs699 - clhs695*clhs922 - clhs701*clhs917 + clhs715*clhs950 + clhs885*clhs950);
const double clhs1132 =             (1.0L/4.0L)*tau2*(clhs250 + clhs254 + clhs255 + clhs259 + clhs264 + clhs267 + clhs270 + clhs273 - clhs736);
const double clhs1133 =             (1.0L/4.0L)*tau2*(clhs278 + clhs282 + clhs284 + clhs285 + clhs288 + clhs291 + clhs293 + clhs296 - clhs740);
const double clhs1134 =             -clhs748 - clhs753;
const double clhs1135 =             clhs1134 + clhs498*clhs726 + clhs498*clhs888 + clhs755*clhs948 + clhs757*clhs948 + clhs759 + clhs761 + clhs763 + clhs765;
const double clhs1136 =             clhs749 + clhs754 - clhs756 - clhs758 - clhs760 - clhs762 - clhs764 - clhs766 - clhs768 - clhs769;
const double clhs1137 =             clhs1136 + clhs743;
const double clhs1138 =             3*N[2]*clhs102*clhs345*v_sc;
const double clhs1139 =             3*N[2]*clhs345*clhs352*clhs7*mu;
const double clhs1140 =             3*N[2]*k_sc;
const double clhs1141 =             3*N[2]*clhs62*clhs7;
const double clhs1142 =             -clhs1055*clhs767 + clhs1057*clhs767 - clhs1138*clhs8 + clhs1139*clhs8 - clhs1140*clhs966 - clhs721*clhs962 - clhs727*clhs963 + clhs969*(-DN(2,0)*clhs475 - clhs1141*clhs51 - clhs1141*clhs55 + clhs249*clhs965 + clhs261*clhs948 + clhs304*clhs395 + clhs51*clhs794 + clhs55*clhs794 - clhs745);
const double clhs1143 =             3*N[2]*clhs46*clhs7;
const double clhs1144 =             -clhs10*clhs1138 + clhs10*clhs1139 + clhs1061*clhs725 - clhs1062*clhs725 - clhs1140*clhs978 - clhs721*clhs975 + clhs893*clhs976 + clhs969*(-DN(2,1)*clhs475 - clhs1143*clhs51 - clhs1143*clhs55 + clhs249*clhs977 + clhs257*clhs348 + clhs307*clhs397 + clhs51*clhs801 + clhs55*clhs801 - clhs750);
const double clhs1145 =             clhs18*clhs781;
const double clhs1146 =             3*clhs352*clhs784*mu;
const double clhs1147 =             clhs10*clhs1146;
const double clhs1148 =             clhs114*clhs720;
const double clhs1149 =             -clhs1148;
const double clhs1150 =             clhs983*(clhs1149 + clhs305 + clhs307);
const double clhs1151 =             N[2]*clhs985;
const double clhs1152 =             -clhs1151;
const double clhs1153 =             -clhs1145 + clhs1147 - clhs1150 + clhs1152;
const double clhs1154 =             clhs781*clhs86;
const double clhs1155 =             clhs1146*clhs8;
const double clhs1156 =             clhs890*clhs983;
const double clhs1157 =             clhs249*clhs960;
const double clhs1158 =             clhs1154 + clhs1155 - clhs1156 - clhs1157;
const double clhs1159 =             clhs709 + clhs710 + clhs711 - clhs716 + clhs882;
const double clhs1160 =             (1.0L/2.0L)*clhs1159*clhs7;
const double clhs1161 =             clhs334*clhs909;
const double clhs1162 =             clhs102*clhs265;
const double clhs1163 =             clhs307*clhs400;
const double clhs1164 =             clhs258*clhs479;
const double clhs1165 =             -clhs1161 - clhs1162 - clhs1163 - clhs1164;
const double clhs1166 =             clhs348*clhs793;
const double clhs1167 =             N[2]*clhs53*clhs7;
const double clhs1168 =             clhs1167*clhs48;
const double clhs1169 =             DN(2,0)*clhs539;
const double clhs1170 =             clhs489*clhs706;
const double clhs1171 =             clhs1165 - clhs1166 + clhs1168 - clhs1169 + clhs1170 + clhs747;
const double clhs1172 =             N[2]*clhs992 - clhs535*clhs733 + 3*clhs697 + 3*clhs699 + clhs772;
const double clhs1173 =             (1.0L/2.0L)*clhs102*clhs313*tau2;
const double clhs1174 =             (1.0L/2.0L)*clhs102*clhs776*gamma*tau3;
const double clhs1175 =             -clhs1004*clhs1160 + clhs1093*clhs1130 - clhs1095*(N[2]*clhs1005 - clhs374*clhs773 + 3*clhs693 + 3*clhs694 + clhs879) + clhs1096*clhs774 + clhs878;
const double clhs1176 =             3*clhs352*clhs780*mu;
const double clhs1177 =             clhs112*clhs720;
const double clhs1178 =             -clhs1177;
const double clhs1179 =             clhs1152 + clhs1176*clhs8 - clhs77*clhs785 - clhs983*(clhs1178 + clhs303 + clhs304);
const double clhs1180 =             -clhs726;
const double clhs1181 =             clhs10*clhs1176 + clhs249*clhs973 + clhs47*clhs785 - clhs983*(clhs1180 + clhs256 + clhs316);
const double clhs1182 =             -clhs102*clhs289 - clhs283*clhs479 - clhs304*clhs423 - clhs318*clhs919;
const double clhs1183 =             -DN(2,1)*clhs552 + clhs1167*clhs87 + clhs1182 - clhs281*clhs479 + clhs492*clhs703 + clhs752;
const double clhs1184 =             (1.0L/2.0L)*clhs102*clhs332*tau2;
const double clhs1185 =             (1.0L/2.0L)*clhs905*tau2;
const double clhs1186 =             clhs1108*clhs1113 + clhs248;
const double clhs1187 =             clhs787 + clhs901;
const double clhs1188 =             (1.0L/2.0L)*DN(2,0)*clhs53*tau2;
const double clhs1189 =             (1.0L/2.0L)*DN(2,1)*clhs53*tau2;
const double clhs1190 =             (1.0L/2.0L)*DN(1,0)*tau2;
const double clhs1191 =             (1.0L/2.0L)*DN(1,1)*tau2;
const double clhs1192 =             -clhs564;
const double clhs1193 =             DN(1,0)*tau2;
const double clhs1194 =             -clhs560;
const double clhs1195 =             DN(1,1)*tau2;
const double clhs1196 =             pow(N[1], 2);
const double clhs1197 =             -bdf0*clhs1196;
const double clhs1198 =             DN(1,0)*N[1];
const double clhs1199 =             -clhs1198;
const double clhs1200 =             DN(1,1)*N[1];
const double clhs1201 =             -clhs1200;
const double clhs1202 =             -N[2]*clhs219;
const double clhs1203 =             DN(2,0)*N[1];
const double clhs1204 =             -clhs1203;
const double clhs1205 =             DN(2,1)*N[1];
const double clhs1206 =             -clhs1205;
const double clhs1207 =             -clhs152*(DN(1,0)*DN(2,0) + DN(1,1)*DN(2,1));
const double clhs1208 =             DN(1,1)*clhs352*clhs7*mu;
const double clhs1209 =             (2.0L/3.0L)*DN(1,0)*clhs352*clhs7*mu;
const double clhs1210 =             (1.0L/2.0L)*N[1]*clhs7;
const double clhs1211 =             clhs155 - clhs180 + clhs611;
const double clhs1212 =             clhs225 + clhs227 + clhs572 - clhs575 + clhs593;
const double clhs1213 =             (1.0L/2.0L)*clhs102*clhs1212*tau2;
const double clhs1214 =             (1.0L/2.0L)*DN(1,0)*clhs53*tau3;
const double clhs1215 =             clhs509*clhs674;
const double clhs1216 =             clhs514*clhs684;
const double clhs1217 =             clhs102*clhs1212*tau2;
const double clhs1218 =             clhs181 + clhs609;
const double clhs1219 =             clhs514*clhs674;
const double clhs1220 =             clhs509*clhs684;
const double clhs1221 =             clhs139*clhs7*tau2;
const double clhs1222 =             clhs102*clhs237*tau2;
const double clhs1223 =             DN(1,0)*tau3;
const double clhs1224 =             clhs1196*clhs4;
const double clhs1225 =             N[1]*clhs372*clhs44*tau1;
const double clhs1226 =             clhs185*clhs31;
const double clhs1227 =             clhs154*clhs383;
const double clhs1228 =             (1.0L/2.0L)*N[1]*clhs405*clhs7*tau2;
const double clhs1229 =             clhs184*clhs8;
const double clhs1230 =             clhs1068 - clhs1099*clhs53 + clhs1229*clhs53 - clhs157 + clhs236;
const double clhs1231 =             (1.0L/2.0L)*N[1]*clhs428*clhs7*tau2;
const double clhs1232 =             clhs1197 + clhs237*clhs669;
const double clhs1233 =             clhs660*clhs674;
const double clhs1234 =             clhs664*clhs684;
const double clhs1235 =             N[1]*clhs372*clhs7*tau1;
const double clhs1236 =             clhs1102 + clhs161 + clhs859;
const double clhs1237 =             clhs156 + clhs662;
const double clhs1238 =             (1.0L/2.0L)*N[1]*clhs102*clhs1237*tau2;
const double clhs1239 =             clhs1238*clhs527;
const double clhs1240 =             clhs184 + clhs658;
const double clhs1241 =             (1.0L/2.0L)*N[1]*clhs102*clhs1240*tau2;
const double clhs1242 =             clhs1241*clhs531;
const double clhs1243 =             clhs664*clhs674;
const double clhs1244 =             clhs660*clhs684;
const double clhs1245 =             clhs1212*clhs7*tau2;
const double clhs1246 =             (1.0L/2.0L)*N[1]*clhs102*clhs53*tau2;
const double clhs1247 =             N[1]*N[2];
const double clhs1248 =             clhs1247*clhs4;
const double clhs1249 =             DN(1,1)*N[2];
const double clhs1250 =             clhs1249*clhs8;
const double clhs1251 =             clhs1205*clhs8;
const double clhs1252 =             N[2]*clhs161;
const double clhs1253 =             DN(1,0)*N[2];
const double clhs1254 =             clhs10*clhs1253;
const double clhs1255 =             clhs1254*clhs31;
const double clhs1256 =             clhs10*clhs1203;
const double clhs1257 =             clhs1256*clhs31;
const double clhs1258 =             N[2]*clhs165;
const double clhs1259 =             clhs1258*clhs31;
const double clhs1260 =             clhs595*clhs703;
const double clhs1261 =             clhs154*clhs707;
const double clhs1262 =             clhs10*clhs1249;
const double clhs1263 =             clhs10*clhs1205;
const double clhs1264 =             N[2]*clhs212;
const double clhs1265 =             clhs1253*clhs8;
const double clhs1266 =             clhs1203*clhs8;
const double clhs1267 =             N[2]*clhs209;
const double clhs1268 =             clhs1267*clhs53;
const double clhs1269 =             clhs154*clhs312;
const double clhs1270 =             clhs154*clhs311;
const double clhs1271 =             -clhs1262 - clhs1263 - clhs1264 + clhs1265*clhs53 + clhs1266*clhs53 + clhs1268 + clhs1269 - clhs1270*clhs53;
const double clhs1272 =             clhs1248 - clhs1271*clhs588 - clhs577*(-clhs1250 - clhs1251 - clhs1252 + clhs1255 + clhs1257 + clhs1259 + clhs1260 - clhs1261);
const double clhs1273 =             N[1]*N[2]*clhs102;
const double clhs1274 =             clhs1273*clhs62;
const double clhs1275 =             N[1]*clhs253 + N[2]*clhs158 + N[2]*clhs163 - N[2]*clhs167 - clhs1273*clhs606 + clhs1274*clhs379 + clhs212*clhs725 - clhs235*clhs725;
const double clhs1276 =             clhs1250 + clhs1251 + clhs1252 - clhs1260;
const double clhs1277 =             clhs1273*clhs46;
const double clhs1278 =             clhs1205 + clhs1249 - clhs1277;
const double clhs1279 =             clhs1278*clhs652;
const double clhs1280 =             clhs1203 + clhs1253 - clhs1274;
const double clhs1281 =             clhs1280*clhs655;
const double clhs1282 =             clhs1202 + clhs1279 - clhs1281*clhs31 + clhs648*(-clhs1255 - clhs1257 - clhs1259 + clhs1261 + clhs1276);
const double clhs1283 =             clhs674*clhs781;
const double clhs1284 =             clhs684*clhs785;
const double clhs1285 =             clhs237*clhs790;
const double clhs1286 =             -clhs1271*clhs648 - clhs1278*clhs677 + clhs1280*clhs678;
const double clhs1287 =             clhs674*clhs785;
const double clhs1288 =             clhs684*clhs781;
const double clhs1289 =             DN(1,0)*clhs352*clhs7*mu;
const double clhs1290 =             (2.0L/3.0L)*DN(1,1)*clhs352*clhs7*mu;
const double clhs1291 =             clhs183 - clhs203 + clhs615;
const double clhs1292 =             (1.0L/2.0L)*clhs102*clhs875*tau2;
const double clhs1293 =             (1.0L/2.0L)*DN(1,1)*clhs53*tau3;
const double clhs1294 =             clhs102*clhs218*tau2;
const double clhs1295 =             -clhs165 + clhs242;
const double clhs1296 =             clhs1295 - clhs224 + clhs240 + clhs594 - clhs848;
const double clhs1297 =             clhs102*clhs1296*tau2;
const double clhs1298 =             clhs204 + clhs613;
const double clhs1299 =             DN(1,1)*tau3;
const double clhs1300 =             clhs1196*clhs69;
const double clhs1301 =             clhs31*clhs859;
const double clhs1302 =             clhs154*clhs825;
const double clhs1303 =             clhs1100 + clhs1229 + clhs209;
const double clhs1304 =             clhs1068*clhs53 + clhs1303 + clhs214 - clhs672;
const double clhs1305 =             clhs7*clhs875*tau2;
const double clhs1306 =             clhs165 + clhs185 + clhs857;
const double clhs1307 =             clhs1247*clhs69;
const double clhs1308 =             clhs1254 + clhs1256 + clhs1258 - clhs641*clhs706;
const double clhs1309 =             -clhs1250*clhs31 - clhs1251*clhs31 - clhs1252*clhs31 + clhs1308 + clhs154*clhs880;
const double clhs1310 =             clhs1265 + clhs1266 + clhs1267 - clhs1270;
const double clhs1311 =             clhs1262*clhs53;
const double clhs1312 =             clhs1263*clhs53;
const double clhs1313 =             clhs1264*clhs53;
const double clhs1314 =             clhs1269*clhs53 + clhs1310 - clhs1311 - clhs1312 - clhs1313;
const double clhs1315 =             clhs1307 + clhs1309*clhs588 + clhs1314*clhs577;
const double clhs1316 =             N[1]*clhs281 + N[2]*clhs186 + N[2]*clhs188 - N[2]*clhs191 - clhs1273*clhs866 + clhs1277*clhs822 + clhs209*clhs767 - clhs213*clhs767;
const double clhs1317 =             clhs1278*clhs869 - clhs1280*clhs868 + clhs1314*clhs648;
const double clhs1318 =             clhs1202 - clhs1279*clhs31 + clhs1281 + clhs1309*clhs648;
const double clhs1319 =             clhs332*clhs669;
const double clhs1320 =             clhs609 + clhs675;
const double clhs1321 =             clhs613 + clhs685;
const double clhs1322 =             clhs1052 + clhs618;
const double clhs1323 =             (1.0L/2.0L)*clhs102*clhs1110*gamma*tau3;
const double clhs1324 =             (1.0L/3.0L)*DN(1,0)*clhs102;
const double clhs1325 =             (1.0L/3.0L)*DN(1,1)*clhs102;
const double clhs1326 =             (1.0L/3.0L)*DN(1,1)*clhs7;
const double clhs1327 =             (1.0L/3.0L)*DN(1,0)*clhs7;
const double clhs1328 =             (1.0L/2.0L)*clhs428*clhs53*clhs7*tau2;
const double clhs1329 =             (1.0L/2.0L)*clhs405*clhs53*clhs7*tau2;
const double clhs1330 =             (1.0L/2.0L)*N[1]*clhs102;
const double clhs1331 =             clhs617 + clhs646;
const double clhs1332 =             DN(1,0)*clhs102*clhs967*clhs968*lambda;
const double clhs1333 =             DN(1,1)*clhs102*clhs967*clhs968*lambda;
const double clhs1334 =             N[1]*clhs102*gamma;
const double clhs1335 =             clhs102*clhs1110*gamma*tau3;
const double clhs1336 =             (1.0L/2.0L)*clhs102*clhs405*gamma*tau2;
const double clhs1337 =             (1.0L/2.0L)*clhs102*clhs428*gamma*tau2;
const double clhs1338 =             (1.0L/2.0L)*N[1]*clhs7*tau2;
const double clhs1339 =             clhs211*clhs907;
const double clhs1340 =             clhs229*clhs914;
const double clhs1341 =             DN(1,0)*clhs489;
const double clhs1342 =             -DN(1,0)*clhs10*clhs47*clhs479 - DN(1,1)*clhs492 - clhs1043 + clhs1104 + clhs229*clhs921 + clhs625;
const double clhs1343 =             N[1]*clhs7*(clhs1236 + clhs1306);
const double clhs1344 =             clhs1065 - clhs1067 + clhs1070 + clhs1071;
const double clhs1345 =             -clhs1073 - clhs1074 + clhs1075 + clhs1076;
const double clhs1346 =             (1.0L/2.0L)*N[1]*clhs7*(clhs1069 + clhs1303 + clhs157 + clhs212);
const double clhs1347 =             3*clhs165;
const double clhs1348 =             clhs1079 + clhs1080 + clhs1081 + clhs1082;
const double clhs1349 =             clhs1084 - clhs1086 + clhs1087 - clhs1088 + clhs1348 - clhs621;
const double clhs1350 =             (1.0L/2.0L)*clhs871*tau2;
const double clhs1351 =             (1.0L/2.0L)*N[1]*clhs102*gamma*tau3;
const double clhs1352 =             3*clhs161;
const double clhs1353 =             (1.0L/2.0L)*N[1]*clhs102*gamma*tau2;
const double clhs1354 =             clhs1276 + clhs1308;
const double clhs1355 =             clhs249*clhs620;
const double clhs1356 =             clhs1311*clhs907;
const double clhs1357 =             clhs1312*clhs907;
const double clhs1358 =             clhs1023*clhs1258;
const double clhs1359 =             clhs1268*clhs907;
const double clhs1360 =             clhs1313*clhs907;
const double clhs1361 =             clhs1027*clhs1252;
const double clhs1362 =             12*N[1]*N[2]*clhs53;
const double clhs1363 =             clhs1030*clhs1362;
const double clhs1364 =             clhs1253*clhs489;
const double clhs1365 =             clhs1203*clhs489;
const double clhs1366 =             clhs1125*clhs154;
const double clhs1367 =             clhs1355 - clhs1356 - clhs1357 - clhs1358 - clhs1359 - clhs1360 - clhs1361 + clhs1363 - clhs1364 - clhs1365 - clhs1366;
const double clhs1368 =             clhs249*clhs625;
const double clhs1369 =             -clhs1039*clhs1254 - clhs1039*clhs1256 - clhs1039*clhs1258 - clhs1040*clhs1268 - clhs1040*clhs1313 - clhs1041*clhs1252 + clhs1042*clhs1362 - clhs1129*clhs154 - clhs1205*clhs492 - clhs1249*clhs492 + clhs1368;
const double clhs1370 =             -clhs1018*clhs1354 - clhs1019*clhs1367 - clhs1037*clhs1369 + clhs1247*clhs447 + clhs603*(-clhs10*clhs1355 - clhs1045*clhs1274 - clhs1046*clhs1277 - clhs1250*clhs498 - clhs1251*clhs498 - clhs1252*clhs922 - clhs1254*clhs498 - clhs1256*clhs498 - clhs1258*clhs917 + clhs1268*clhs950 + clhs1313*clhs950 - clhs1368*clhs8);
const double clhs1371 =             clhs1262 + clhs1263 + clhs1264 - clhs1269 + clhs1310;
const double clhs1372 =             (1.0L/2.0L)*clhs1371*clhs7;
const double clhs1373 =             N[2]*clhs1347 + 3*clhs1254 + 3*clhs1256 - clhs1274*clhs535 + clhs1276;
const double clhs1374 =             (1.0L/2.0L)*tau2;
const double clhs1375 =             -clhs1004*clhs1372 + clhs1093*clhs1369 - clhs1095*(N[2]*clhs1352 + 3*clhs1250 + 3*clhs1251 - clhs1277*clhs374 + clhs1308) + clhs1096*clhs1278 + clhs1307;
const double clhs1376 =             clhs1108*clhs1354 + clhs1202;
const double clhs1377 =             (1.0L/2.0L)*DN(2,0)*tau2;
const double clhs1378 =             (1.0L/2.0L)*DN(2,1)*tau2;
const double clhs1379 =             -clhs696;
const double clhs1380 =             DN(2,0)*tau2;
const double clhs1381 =             -clhs692;
const double clhs1382 =             DN(2,1)*tau2;
const double clhs1383 =             -clhs1253;
const double clhs1384 =             -clhs1249;
const double clhs1385 =             pow(N[2], 2);
const double clhs1386 =             -bdf0*clhs1385;
const double clhs1387 =             DN(2,0)*N[2];
const double clhs1388 =             -clhs1387;
const double clhs1389 =             DN(2,1)*N[2];
const double clhs1390 =             -clhs1389;
const double clhs1391 =             DN(2,1)*clhs352*clhs7*mu;
const double clhs1392 =             (2.0L/3.0L)*DN(2,0)*clhs352*clhs7*mu;
const double clhs1393 =             (1.0L/2.0L)*N[2]*clhs7;
const double clhs1394 =             clhs250 - clhs275 + clhs737;
const double clhs1395 =             clhs320 + clhs322 + clhs704 - clhs707 + clhs723;
const double clhs1396 =             (1.0L/2.0L)*clhs102*clhs1395*tau2;
const double clhs1397 =             (1.0L/2.0L)*DN(2,0)*clhs53*tau3;
const double clhs1398 =             clhs509*clhs794;
const double clhs1399 =             clhs514*clhs801;
const double clhs1400 =             clhs102*clhs1395*tau2;
const double clhs1401 =             clhs276 + clhs735;
const double clhs1402 =             clhs514*clhs794;
const double clhs1403 =             clhs509*clhs801;
const double clhs1404 =             clhs102*clhs332*tau2;
const double clhs1405 =             DN(2,0)*tau3;
const double clhs1406 =             clhs660*clhs794;
const double clhs1407 =             clhs664*clhs801;
const double clhs1408 =             clhs664*clhs794;
const double clhs1409 =             clhs660*clhs801;
const double clhs1410 =             clhs1395*clhs7*tau2;
const double clhs1411 =             clhs1385*clhs4;
const double clhs1412 =             N[2]*clhs372*clhs44*tau1;
const double clhs1413 =             clhs280*clhs31;
const double clhs1414 =             clhs31*clhs888;
const double clhs1415 =             (1.0L/2.0L)*N[2]*clhs405*clhs7*tau2;
const double clhs1416 =             clhs279*clhs8;
const double clhs1417 =             clhs1148 - clhs1177*clhs53 + clhs1416*clhs53 - clhs252 + clhs331;
const double clhs1418 =             (1.0L/2.0L)*N[2]*clhs428*clhs7*tau2;
const double clhs1419 =             clhs1386 + clhs332*clhs790;
const double clhs1420 =             clhs781*clhs794;
const double clhs1421 =             clhs785*clhs801;
const double clhs1422 =             N[2]*clhs372*clhs7*tau1;
const double clhs1423 =             clhs1180 + clhs256 + clhs891;
const double clhs1424 =             clhs251 + clhs783;
const double clhs1425 =             (1.0L/2.0L)*N[2]*clhs102*clhs1424*tau2;
const double clhs1426 =             clhs1425*clhs527;
const double clhs1427 =             clhs279 + clhs779;
const double clhs1428 =             (1.0L/2.0L)*N[2]*clhs102*clhs1427*tau2;
const double clhs1429 =             clhs1428*clhs531;
const double clhs1430 =             clhs785*clhs794;
const double clhs1431 =             clhs781*clhs801;
const double clhs1432 =             (1.0L/2.0L)*N[2]*clhs102*clhs53*tau2;
const double clhs1433 =             DN(2,0)*clhs352*clhs7*mu;
const double clhs1434 =             (2.0L/3.0L)*DN(2,1)*clhs352*clhs7*mu;
const double clhs1435 =             clhs278 - clhs298 + clhs741;
const double clhs1436 =             (1.0L/2.0L)*clhs102*clhs903*tau2;
const double clhs1437 =             (1.0L/2.0L)*DN(2,1)*clhs53*tau3;
const double clhs1438 =             clhs102*clhs313*tau2;
const double clhs1439 =             -clhs260 + clhs337;
const double clhs1440 =             clhs1439 - clhs319 + clhs335 + clhs724 - clhs880;
const double clhs1441 =             clhs102*clhs1440*tau2;
const double clhs1442 =             clhs299 + clhs739;
const double clhs1443 =             DN(2,1)*tau3;
const double clhs1444 =             clhs1385*clhs69;
const double clhs1445 =             clhs31*clhs891;
const double clhs1446 =             clhs31*clhs726;
const double clhs1447 =             clhs1178 + clhs1416 + clhs304;
const double clhs1448 =             clhs1148*clhs53 + clhs1447 + clhs309 - clhs793;
const double clhs1449 =             clhs260 + clhs280 + clhs889;
const double clhs1450 =             clhs735 + clhs795;
const double clhs1451 =             clhs739 + clhs802;
const double clhs1452 =             clhs1136 + clhs744;
const double clhs1453 =             (1.0L/2.0L)*clhs102*clhs1187*gamma*tau3;
const double clhs1454 =             (1.0L/3.0L)*DN(2,0)*clhs102;
const double clhs1455 =             (1.0L/3.0L)*DN(2,1)*clhs102;
const double clhs1456 =             (1.0L/3.0L)*DN(2,1)*clhs7;
const double clhs1457 =             (1.0L/3.0L)*DN(2,0)*clhs7;
const double clhs1458 =             (1.0L/2.0L)*N[2]*clhs102;
const double clhs1459 =             clhs743 + clhs770;
const double clhs1460 =             DN(2,0)*clhs102*clhs967*clhs968*lambda;
const double clhs1461 =             DN(2,1)*clhs102*clhs967*clhs968*lambda;
const double clhs1462 =             N[2]*clhs102*gamma;
const double clhs1463 =             clhs102*clhs1187*gamma*tau3;
const double clhs1464 =             (1.0L/2.0L)*N[2]*clhs7*tau2;
const double clhs1465 =             clhs306*clhs907;
const double clhs1466 =             clhs324*clhs914;
const double clhs1467 =             DN(2,0)*clhs489;
const double clhs1468 =             -DN(2,0)*clhs10*clhs47*clhs479 - DN(2,1)*clhs492 - clhs1129 + clhs1182 + clhs324*clhs921 + clhs751;
const double clhs1469 =             N[2]*clhs7*(clhs1423 + clhs1449);
const double clhs1470 =             (1.0L/2.0L)*N[2]*clhs7*(clhs1149 + clhs1447 + clhs252 + clhs307);
const double clhs1471 =             clhs1161 + clhs1162 + clhs1163 + clhs1164;
const double clhs1472 =             (1.0L/2.0L)*N[2]*clhs102*gamma*tau3;
const double clhs1473 =             (1.0L/2.0L)*N[2]*clhs102*gamma*tau2;
            lhs(0,0)=clhs1 + clhs2*clhs67 + clhs68*clhs99;
            lhs(0,1)=clhs101 - clhs103*clhs116 + clhs117*clhs133;
            lhs(0,2)=clhs135 + clhs136*clhs139 - clhs140*clhs151;
            lhs(0,3)=-clhs152*(pow(DN(0,0), 2) + pow(DN(0,1), 2));
            lhs(0,4)=clhs153 + clhs182*clhs2 + clhs205*clhs68;
            lhs(0,5)=-clhs103*clhs218 + clhs117*clhs232 + clhs207;
            lhs(0,6)=clhs136*clhs237 - clhs140*clhs246 + clhs234;
            lhs(0,7)=clhs247;
            lhs(0,8)=clhs2*clhs277 + clhs248 + clhs300*clhs68;
            lhs(0,9)=-clhs103*clhs313 + clhs117*clhs327 + clhs302;
            lhs(0,10)=clhs136*clhs332 - clhs140*clhs341 + clhs329;
            lhs(0,11)=clhs342;
            lhs(1,0)=clhs100*clhs351 + clhs134*clhs346 + clhs343 + clhs353*clhs356 + clhs357*clhs363 - clhs364*clhs368 - clhs369*clhs371 - clhs373*(DN(0,0)*clhs376 + clhs106*clhs47 - clhs115*clhs374 + clhs23 - clhs33 + clhs360*clhs379 + clhs365 - clhs367 - clhs59) - clhs406*(clhs126 + clhs358 + clhs362 + clhs381 + clhs382 - clhs384) - clhs410*clhs429 + clhs434*clhs438 + clhs439*clhs445 - clhs446*clhs503;
            lhs(1,1)=-clhs111*clhs518 - clhs133*clhs522 - clhs31*clhs532 + clhs446*clhs542 + clhs505 - 4.0L/3.0L*clhs510 - clhs515 + clhs519*(-clhs382 + clhs384 + clhs517 + clhs521) + clhs523*clhs524 + clhs528;
            lhs(1,2)=clhs111*clhs139 - clhs139*clhs545 + clhs151*clhs546 - clhs371*clhs547 - clhs405*clhs526 - clhs410*clhs519 + clhs428*clhs529*clhs548 + clhs446*clhs555 + (2.0L/3.0L)*clhs543 - clhs544;
            lhs(1,3)=clhs53*(clhs101 + clhs103*clhs139 + clhs136*clhs433 - clhs556*clhs557);
            lhs(1,4)=clhs346*clhs560 + clhs351*clhs564 + clhs353*clhs591 + clhs357*clhs598 - clhs364*clhs601 - clhs371*clhs602 + clhs434*clhs612 + clhs439*clhs616 - clhs446*clhs647 + clhs589 - clhs603*(-clhs206*clhs58 + clhs376*clhs564 + clhs608);
            lhs(1,5)=-clhs111*clhs668 - clhs232*clhs522 + clhs446*clhs676 + clhs524*clhs671 + clhs657 - 4.0L/3.0L*clhs661 - clhs665 + clhs670;
            lhs(1,6)=clhs111*clhs237 - clhs237*clhs545 + clhs246*clhs546 - clhs371*clhs682 + clhs446*clhs686 + clhs679 + (2.0L/3.0L)*clhs680 - clhs681;
            lhs(1,7)=clhs53*(clhs139*clhs688 + clhs207 + clhs433*clhs687 - clhs556*clhs689);
            lhs(1,8)=clhs346*clhs692 + clhs351*clhs696 + clhs353*clhs721 + clhs357*clhs727 - clhs364*clhs730 - clhs371*clhs731 + clhs434*clhs738 + clhs439*clhs742 - clhs446*clhs771 - clhs603*(-clhs301*clhs58 + clhs376*clhs696 + clhs734) + clhs719;
            lhs(1,9)=-clhs111*clhs789 - clhs327*clhs522 + clhs446*clhs796 + clhs524*clhs792 + clhs778 - 4.0L/3.0L*clhs782 - clhs786 + clhs791;
            lhs(1,10)=clhs111*clhs332 - clhs332*clhs545 + clhs341*clhs546 - clhs371*clhs800 + clhs446*clhs803 + clhs797 + (2.0L/3.0L)*clhs798 - clhs799;
            lhs(1,11)=clhs53*(clhs139*clhs805 + clhs302 + clhs433*clhs804 - clhs556*clhs806);
            lhs(2,0)=clhs100*clhs346 - clhs134*clhs810 + clhs356*clhs811 - clhs364*clhs820 - clhs369*clhs821 - clhs373*(DN(0,1)*clhs441 - clhs113*clhs535 + clhs123*clhs47 + clhs430*clhs822 + clhs75 - clhs80 + clhs817 - clhs819 - clhs92) + clhs406*clhs829 - clhs429*(-clhs72 + clhs813 + clhs823 + clhs824 - clhs826) - clhs438*clhs835 - clhs445*clhs834 - clhs503*clhs836 + clhs807 - clhs812*clhs816;
            lhs(2,1)=-clhs111*clhs116 - clhs116*clhs837 + clhs405*clhs525*clhs548 - clhs428*clhs530 + clhs519*clhs829 - clhs523*clhs821 + clhs542*clhs836 - clhs543 + (2.0L/3.0L)*clhs544 - clhs838*clhs839;
            lhs(2,2)=-clhs111*clhs833 - clhs31*clhs528 + clhs505 - clhs510 - 4.0L/3.0L*clhs515 + clhs519*(-clhs824 + clhs826 + clhs831 + clhs840) + clhs532 + clhs547*clhs844 + clhs555*clhs836 - clhs842*clhs843;
            lhs(2,3)=clhs53*(clhs103*clhs841 - clhs116*clhs136 + clhs135 - clhs557*clhs845);
            lhs(2,4)=clhs346*clhs564 - clhs364*clhs864 - clhs560*clhs810 + clhs591*clhs811 - clhs602*clhs821 - clhs603*(-clhs233*clhs91 + clhs441*clhs560 + clhs867) - clhs612*clhs835 - clhs616*clhs834 - clhs647*clhs836 - clhs812*clhs861 + clhs855;
            lhs(2,5)=-clhs111*clhs218 - clhs218*clhs837 - clhs671*clhs821 + clhs676*clhs836 - clhs680 + (2.0L/3.0L)*clhs681 - clhs838*clhs871 + clhs870;
            lhs(2,6)=-clhs111*clhs875 - clhs661 - 4.0L/3.0L*clhs665 + clhs682*clhs844 + clhs686*clhs836 - clhs842*clhs877 + clhs872 + clhs876;
            lhs(2,7)=clhs53*(-clhs116*clhs687 + clhs234 + clhs688*clhs841 - clhs689*clhs845);
            lhs(2,8)=clhs346*clhs696 - clhs364*clhs896 - clhs603*(-clhs328*clhs91 + clhs441*clhs692 + clhs897) - clhs692*clhs810 + clhs721*clhs811 - clhs731*clhs821 - clhs738*clhs835 - clhs742*clhs834 - clhs771*clhs836 - clhs812*clhs893 + clhs887;
            lhs(2,9)=-clhs111*clhs313 - clhs313*clhs837 - clhs792*clhs821 + clhs796*clhs836 - clhs798 + (2.0L/3.0L)*clhs799 - clhs838*clhs899 + clhs898;
            lhs(2,10)=-clhs111*clhs903 - clhs782 - 4.0L/3.0L*clhs786 + clhs800*clhs844 + clhs803*clhs836 - clhs842*clhs905 + clhs900 + clhs904;
            lhs(2,11)=clhs53*(-clhs116*clhs804 + clhs329 + clhs805*clhs841 - clhs806*clhs845);
            lhs(3,0)=clhs0*clhs447 - clhs364*clhs949 + clhs369*clhs953 + clhs373*(clhs109*clhs950 + clhs137*clhs950 - clhs21*clhs922 - clhs29*clhs917 - clhs361*clhs951 - clhs380*clhs498 - clhs431*clhs951 - clhs498*clhs72 + clhs947) - clhs527*clhs906*clhs923 - clhs531*clhs906*(clhs451 - clhs908 - clhs910 - clhs911 - clhs912 - clhs913 + clhs915 - clhs916 - clhs918) - clhs924*clhs925 - clhs926*clhs927 - clhs928*clhs946 + clhs955*clhs956 - clhs957*clhs971 - clhs972*clhs980;
            lhs(3,1)=(1.0L/2.0L)*N[0]*clhs7*clhs993*(6*clhs123 - 6*clhs361 + clhs521 + clhs992) - clhs1000*clhs529*clhs999 + clhs343 - clhs519*(-clhs451 + clhs908 - clhs915 + clhs916 + clhs918 + clhs995) - clhs523*(clhs448 + clhs502) + clhs542*clhs955 + clhs835*clhs927 + clhs925*clhs998 + clhs981*clhs987 + clhs988*clhs989 + clhs990*clhs991 + clhs994*clhs997;
            lhs(3,2)=-1.0L/2.0L*N[0]*clhs1006*clhs7*(clhs1005 + 6*clhs120 - 6*clhs431 + clhs840) - clhs1002*clhs988 - clhs1003*clhs981 - clhs1004*clhs990 - clhs1007*clhs994 - clhs1008*clhs927 - clhs439*clhs925 + clhs519*clhs923 + clhs525*clhs945*clhs999 + clhs547*clhs953 + clhs555*clhs955 + clhs807;
            lhs(3,3)=clhs1 - clhs1009*clhs508 - clhs1010*clhs513 - clhs1011*clhs954 + clhs1012*clhs928 - clhs1013*clhs557 - clhs1014*clhs405*clhs529 - clhs1014*clhs428*clhs525 + clhs1015*clhs925 + clhs1016*clhs927;
            lhs(3,4)=clhs1047 - clhs1048*clhs925 - clhs1049*clhs927 - clhs1051*clhs364 + clhs1053*clhs955 - clhs1060*clhs957 - clhs1064*clhs972 + clhs602*clhs953;
            lhs(3,5)=-clhs1006*clhs1078 + clhs1036*clhs1093 - clhs1089*clhs1090 + clhs1091*clhs927 - clhs1092*clhs232 + clhs1094*clhs945 + clhs559 + clhs671*clhs953 + clhs676*clhs955 - clhs981*(-clhs1065 + clhs1067 - clhs1070 + clhs1072) - clhs988*(clhs1073 + clhs1074 - clhs1075 - clhs1076) - clhs994*(clhs1083 - clhs1084 + clhs1086 - clhs1087 + clhs1088 + clhs621);
            lhs(3,6)=clhs1097 - clhs1101*clhs988 - clhs1103*clhs981 - clhs1105*clhs994 - clhs1106*clhs925 - clhs1107*clhs927 + clhs682*clhs953 + clhs686*clhs955;
            lhs(3,7)=-clhs1009*clhs659 - clhs1010*clhs663 - clhs1011*clhs1110 - clhs1013*clhs689 + clhs1109 + clhs1111*clhs925 + clhs1112*clhs927 + clhs653*gamma + clhs656*gamma;
            lhs(3,8)=clhs1131 - clhs1132*clhs925 - clhs1133*clhs927 - clhs1135*clhs364 + clhs1137*clhs955 - clhs1142*clhs957 - clhs1144*clhs972 + clhs731*clhs953;
            lhs(3,9)=-clhs1006*clhs1160 - clhs1090*clhs1172 - clhs1092*clhs327 + clhs1093*clhs1127 - clhs1153*clhs981 - clhs1158*clhs988 - clhs1171*clhs994 + clhs1173*clhs927 + clhs1174*clhs945 + clhs691 + clhs792*clhs953 + clhs796*clhs955;
            lhs(3,10)=clhs1175 - clhs1179*clhs988 - clhs1181*clhs981 - clhs1183*clhs994 - clhs1184*clhs925 - clhs1185*clhs927 + clhs800*clhs953 + clhs803*clhs955;
            lhs(3,11)=-clhs1009*clhs780 - clhs1010*clhs784 - clhs1011*clhs1187 - clhs1013*clhs806 + clhs1186 + clhs1188*clhs925 + clhs1189*clhs927 + clhs775*gamma + clhs777*gamma;
            lhs(4,0)=clhs1190*clhs67 + clhs1191*clhs99 + clhs153;
            lhs(4,1)=-clhs116*clhs688 + clhs1192 + clhs1193*clhs133;
            lhs(4,2)=clhs1194 - clhs1195*clhs151 + clhs139*clhs687;
            lhs(4,3)=clhs247;
            lhs(4,4)=clhs1190*clhs182 + clhs1191*clhs205 + clhs1197;
            lhs(4,5)=clhs1193*clhs232 + clhs1199 - clhs218*clhs688;
            lhs(4,6)=-clhs1195*clhs246 + clhs1201 + clhs237*clhs687;
            lhs(4,7)=-clhs152*(pow(DN(1,0), 2) + pow(DN(1,1), 2));
            lhs(4,8)=clhs1190*clhs277 + clhs1191*clhs300 + clhs1202;
            lhs(4,9)=clhs1193*clhs327 + clhs1204 - clhs313*clhs688;
            lhs(4,10)=-clhs1195*clhs341 + clhs1206 + clhs332*clhs687;
            lhs(4,11)=clhs1207;
            lhs(5,0)=clhs1106*clhs445 + clhs1208*clhs356 + clhs1209*clhs363 - clhs1210*clhs368 - clhs1211*clhs369 + clhs1213*clhs438 - clhs1214*clhs503 + clhs206*clhs351 + clhs233*clhs346 + clhs589 - clhs603*(clhs206*clhs376 - clhs564*clhs58 + clhs608);
            lhs(5,1)=clhs1214*clhs542 - 4.0L/3.0L*clhs1215 - clhs1216 - clhs1217*clhs133 + clhs1218*clhs523 - clhs215*clhs518 + clhs657 + clhs876;
            lhs(5,2)=-clhs1211*clhs547 - clhs1212*clhs1221 + clhs1214*clhs555 + (2.0L/3.0L)*clhs1219 - clhs1220 + clhs1222*clhs151 + clhs139*clhs215 + clhs679;
            lhs(5,3)=clhs53*(clhs103*clhs237 + clhs1192 + clhs1212*clhs136 - clhs1223*clhs557);
            lhs(5,4)=clhs1106*clhs616 + clhs1198*clhs351 + clhs1200*clhs346 + clhs1208*clhs591 + clhs1209*clhs598 - clhs1210*clhs601 - clhs1211*clhs602 + clhs1213*clhs612 - clhs1214*clhs647 + clhs1224 - clhs1225*(DN(1,0)*clhs376 + clhs163 - clhs167 - clhs176 + clhs210*clhs47 - clhs217*clhs374 + clhs379*clhs574 + clhs599 - clhs600) - clhs1228*(clhs1226 - clhs1227 + clhs227 + clhs592 + clhs597 + clhs860) - clhs1230*clhs1231;
            lhs(5,5)=clhs1214*clhs676 - clhs1217*clhs232 + clhs1218*clhs671 + clhs1232 - 4.0L/3.0L*clhs1233 - clhs1234 + clhs1235*(-clhs1226 + clhs1227 + clhs1236 + clhs667) + clhs1239 - clhs1242*clhs31 - clhs215*clhs668;
            lhs(5,6)=-clhs1211*clhs682 + clhs1214*clhs686 + clhs1222*clhs246 - clhs1230*clhs1235 - clhs1238*clhs405 + clhs1240*clhs1246*clhs428 + (2.0L/3.0L)*clhs1243 - clhs1244 - clhs1245*clhs237 + clhs215*clhs237;
            lhs(5,7)=clhs53*(clhs1199 + clhs1212*clhs687 - clhs1223*clhs689 + clhs237*clhs688);
            lhs(5,8)=clhs1106*clhs742 + clhs1208*clhs721 + clhs1209*clhs727 - clhs1210*clhs730 - clhs1211*clhs731 + clhs1213*clhs738 - clhs1214*clhs771 + clhs1249*clhs346 + clhs1253*clhs351 + clhs1272 - clhs603*(-clhs1203*clhs58 + clhs1253*clhs376 + clhs1275);
            lhs(5,9)=clhs1214*clhs796 - clhs1217*clhs327 + clhs1218*clhs792 + clhs1282 - 4.0L/3.0L*clhs1283 - clhs1284 + clhs1285 - clhs215*clhs789;
            lhs(5,10)=-clhs1211*clhs800 + clhs1214*clhs803 + clhs1222*clhs341 - clhs1245*clhs332 + clhs1286 + (2.0L/3.0L)*clhs1287 - clhs1288 + clhs215*clhs332;
            lhs(5,11)=clhs53*(clhs1204 + clhs1212*clhs804 - clhs1223*clhs806 + clhs237*clhs805);
            lhs(6,0)=-clhs1091*clhs438 - clhs1210*clhs820 + clhs1289*clhs356 - clhs1290*clhs816 - clhs1291*clhs369 - clhs1292*clhs445 - clhs1293*clhs503 + clhs206*clhs346 - clhs233*clhs810 - clhs603*(clhs233*clhs441 - clhs560*clhs91 + clhs867) + clhs855;
            lhs(6,1)=-clhs116*clhs215 - clhs1219 + (2.0L/3.0L)*clhs1220 - clhs1291*clhs523 + clhs1293*clhs542 - clhs1294*clhs839 - clhs504*clhs875 + clhs870;
            lhs(6,2)=-clhs1215 - 4.0L/3.0L*clhs1216 + clhs1293*clhs555 - clhs1297*clhs843 + clhs1298*clhs547 - clhs215*clhs833 + clhs670 + clhs872;
            lhs(6,3)=clhs53*(clhs103*clhs1296 + clhs1194 - clhs1299*clhs557 - clhs136*clhs218);
            lhs(6,4)=-clhs1091*clhs612 + clhs1198*clhs346 - clhs1200*clhs810 - clhs1210*clhs864 - clhs1225*(DN(1,1)*clhs441 + clhs188 - clhs191 - clhs199 - clhs216*clhs535 + clhs224*clhs47 + clhs571*clhs822 + clhs862 - clhs863) + clhs1228*clhs1304 - clhs1231*(clhs1295 + clhs1301 - clhs1302 - clhs185 + clhs856) + clhs1289*clhs591 - clhs1290*clhs861 - clhs1291*clhs602 - clhs1292*clhs616 - clhs1293*clhs647 + clhs1300;
            lhs(6,5)=clhs1235*clhs1304 + clhs1237*clhs1246*clhs405 - clhs1241*clhs428 - clhs1243 + (2.0L/3.0L)*clhs1244 - clhs1291*clhs671 + clhs1293*clhs676 - clhs1294*clhs871 - clhs1305*clhs218 - clhs215*clhs218;
            lhs(6,6)=clhs1232 - clhs1233 - 4.0L/3.0L*clhs1234 + clhs1235*(-clhs1301 + clhs1302 + clhs1306 + clhs874) - clhs1239*clhs31 + clhs1242 + clhs1293*clhs686 - clhs1297*clhs877 + clhs1298*clhs682 - clhs215*clhs875;
            lhs(6,7)=clhs53*(clhs1201 + clhs1296*clhs688 - clhs1299*clhs689 - clhs218*clhs687);
            lhs(6,8)=-clhs1091*clhs738 - clhs1210*clhs896 - clhs1249*clhs810 + clhs1253*clhs346 + clhs1289*clhs721 - clhs1290*clhs893 - clhs1291*clhs731 - clhs1292*clhs742 - clhs1293*clhs771 + clhs1315 - clhs603*(-clhs1205*clhs91 + clhs1249*clhs441 + clhs1316);
            lhs(6,9)=-clhs1287 + (2.0L/3.0L)*clhs1288 - clhs1291*clhs792 + clhs1293*clhs796 - clhs1294*clhs899 - clhs1305*clhs313 + clhs1317 - clhs215*clhs313;
            lhs(6,10)=-clhs1283 - 4.0L/3.0L*clhs1284 + clhs1293*clhs803 - clhs1297*clhs905 + clhs1298*clhs800 + clhs1318 + clhs1319 - clhs215*clhs903;
            lhs(6,11)=clhs53*(clhs1206 + clhs1296*clhs805 - clhs1299*clhs806 - clhs218*clhs804);
            lhs(7,0)=clhs1047 - clhs1210*clhs949 - clhs1320*clhs924 - clhs1321*clhs926 + clhs1322*clhs369 + clhs1323*clhs956 - clhs1324*clhs971 - clhs1325*clhs980;
            lhs(7,1)=-clhs1000*clhs1094 + clhs1077*clhs1328 + clhs1089*clhs1329 - clhs1093*(-clhs1020 + clhs1021 + clhs1022 + clhs1024 + clhs1025 + clhs1026 + clhs1028 - clhs1031 + clhs1032 + clhs1033 + clhs1035) + clhs1320*clhs998 + clhs1321*clhs835 + clhs1323*clhs542 + clhs1326*clhs987 + clhs1327*clhs989 + clhs1330*clhs997 - clhs1331*clhs523 + clhs559;
            lhs(7,2)=-clhs1002*clhs1327 - clhs1003*clhs1326 - clhs1007*clhs1330 - clhs1008*clhs1321 + clhs1097 - clhs1320*clhs439 + clhs1322*clhs547 + clhs1323*clhs555;
            lhs(7,3)=clhs1015*clhs1320 + clhs1016*clhs1321 + clhs1109 - clhs1332*clhs508 - clhs1333*clhs513 - clhs1334*clhs954 - clhs1335*clhs557 - clhs1336*clhs654 - clhs1337*clhs651;
            lhs(7,4)=-clhs1048*clhs1320 - clhs1049*clhs1321 - clhs1051*clhs1210 + clhs1053*clhs1323 - clhs1060*clhs1324 - clhs1064*clhs1325 + clhs1196*clhs447 + clhs1225*(clhs1050 - clhs161*clhs922 - clhs165*clhs917 - clhs185*clhs498 + clhs213*clhs950 + clhs235*clhs950 - clhs498*clhs859 - clhs572*clhs951 - clhs594*clhs951) + clhs1322*clhs602 - clhs1338*clhs1342*clhs527 - clhs1338*clhs531*(-clhs1034 + clhs1083 - clhs1339 + clhs1340 - clhs1341 + clhs620) - clhs1343*clhs946;
            lhs(7,5)=(1.0L/2.0L)*N[1]*clhs7*clhs993*(clhs1236 + clhs1347 + 6*clhs224 - 6*clhs594) - clhs1000*clhs1240*clhs1351 + clhs1091*clhs1321 + clhs1224 - clhs1235*(clhs1034 + clhs1339 - clhs1340 + clhs1341 + clhs1348 - clhs620) + clhs1320*clhs1350 + clhs1323*clhs676 + clhs1326*clhs1344 + clhs1327*clhs1345 + clhs1330*clhs1349 - clhs1331*clhs671 + clhs1346*clhs991;
            lhs(7,6)=-1.0L/2.0L*N[1]*clhs1006*clhs7*(clhs1306 + clhs1352 + 6*clhs221 - 6*clhs572) - clhs1004*clhs1346 - clhs1101*clhs1327 - clhs1103*clhs1326 - clhs1105*clhs1330 - clhs1106*clhs1320 - clhs1107*clhs1321 + clhs1235*clhs1342 + clhs1237*clhs1351*clhs945 + clhs1300 + clhs1322*clhs682 + clhs1323*clhs686;
            lhs(7,7)=clhs1012*clhs1343 - clhs1110*clhs1334 + clhs1111*clhs1320 + clhs1112*clhs1321 + clhs1197 - clhs1237*clhs1353*clhs428 - clhs1240*clhs1353*clhs405 - clhs1332*clhs659 - clhs1333*clhs663 - clhs1335*clhs689;
            lhs(7,8)=-clhs1132*clhs1320 - clhs1133*clhs1321 - clhs1135*clhs1210 + clhs1137*clhs1323 - clhs1142*clhs1324 - clhs1144*clhs1325 + clhs1322*clhs731 + clhs1370;
            lhs(7,9)=-clhs1006*clhs1372 - clhs1090*clhs1373 + clhs1093*clhs1367 + clhs1096*clhs1280 - clhs1153*clhs1326 - clhs1158*clhs1327 - clhs1171*clhs1330 + clhs1173*clhs1321 + clhs1248 - clhs1320*clhs1374*clhs327 + clhs1322*clhs792 + clhs1323*clhs796;
            lhs(7,10)=-clhs1179*clhs1327 - clhs1181*clhs1326 - clhs1183*clhs1330 - clhs1184*clhs1320 - clhs1185*clhs1321 + clhs1322*clhs800 + clhs1323*clhs803 + clhs1375;
            lhs(7,11)=-clhs1187*clhs1334 + clhs1188*clhs1320 + clhs1189*clhs1321 + clhs1279*gamma + clhs1281*gamma - clhs1332*clhs780 - clhs1333*clhs784 - clhs1335*clhs806 + clhs1376;
            lhs(8,0)=clhs1377*clhs67 + clhs1378*clhs99 + clhs248;
            lhs(8,1)=-clhs116*clhs805 + clhs133*clhs1380 + clhs1379;
            lhs(8,2)=clhs1381 - clhs1382*clhs151 + clhs139*clhs804;
            lhs(8,3)=clhs342;
            lhs(8,4)=clhs1202 + clhs1377*clhs182 + clhs1378*clhs205;
            lhs(8,5)=clhs1380*clhs232 + clhs1383 - clhs218*clhs805;
            lhs(8,6)=-clhs1382*clhs246 + clhs1384 + clhs237*clhs804;
            lhs(8,7)=clhs1207;
            lhs(8,8)=clhs1377*clhs277 + clhs1378*clhs300 + clhs1386;
            lhs(8,9)=clhs1380*clhs327 + clhs1388 - clhs313*clhs805;
            lhs(8,10)=-clhs1382*clhs341 + clhs1390 + clhs332*clhs804;
            lhs(8,11)=-clhs152*(pow(DN(2,0), 2) + pow(DN(2,1), 2));
            lhs(9,0)=clhs1184*clhs445 + clhs1391*clhs356 + clhs1392*clhs363 - clhs1393*clhs368 - clhs1394*clhs369 + clhs1396*clhs438 - clhs1397*clhs503 + clhs301*clhs351 + clhs328*clhs346 - clhs603*(clhs301*clhs376 - clhs58*clhs696 + clhs734) + clhs719;
            lhs(9,1)=-clhs133*clhs1400 + clhs1397*clhs542 - 4.0L/3.0L*clhs1398 - clhs1399 + clhs1401*clhs523 - clhs310*clhs518 + clhs778 + clhs904;
            lhs(9,2)=-clhs1221*clhs1395 + clhs139*clhs310 - clhs1394*clhs547 + clhs1397*clhs555 + (2.0L/3.0L)*clhs1402 - clhs1403 + clhs1404*clhs151 + clhs797;
            lhs(9,3)=clhs53*(clhs103*clhs332 + clhs136*clhs1395 + clhs1379 - clhs1405*clhs557);
            lhs(9,4)=clhs1184*clhs616 + clhs1203*clhs351 + clhs1205*clhs346 + clhs1272 + clhs1391*clhs591 + clhs1392*clhs598 - clhs1393*clhs601 - clhs1394*clhs602 + clhs1396*clhs612 - clhs1397*clhs647 - clhs603*(clhs1203*clhs376 - clhs1253*clhs58 + clhs1275);
            lhs(9,5)=clhs1282 + clhs1319 + clhs1397*clhs676 - clhs1400*clhs232 + clhs1401*clhs671 - 4.0L/3.0L*clhs1406 - clhs1407 - clhs310*clhs668;
            lhs(9,6)=clhs1286 - clhs1394*clhs682 + clhs1397*clhs686 + clhs1404*clhs246 + (2.0L/3.0L)*clhs1408 - clhs1409 - clhs1410*clhs237 + clhs237*clhs310;
            lhs(9,7)=clhs53*(clhs1383 + clhs1395*clhs687 - clhs1405*clhs689 + clhs332*clhs688);
            lhs(9,8)=clhs1184*clhs742 + clhs1387*clhs351 + clhs1389*clhs346 + clhs1391*clhs721 + clhs1392*clhs727 - clhs1393*clhs730 - clhs1394*clhs731 + clhs1396*clhs738 - clhs1397*clhs771 + clhs1411 - clhs1412*(DN(2,0)*clhs376 + clhs258 - clhs262 - clhs271 + clhs305*clhs47 - clhs312*clhs374 + clhs379*clhs706 + clhs728 - clhs729) - clhs1415*(clhs1413 - clhs1414 + clhs322 + clhs722 + clhs726 + clhs892) - clhs1417*clhs1418;
            lhs(9,9)=clhs1397*clhs796 - clhs1400*clhs327 + clhs1401*clhs792 + clhs1419 - 4.0L/3.0L*clhs1420 - clhs1421 + clhs1422*(-clhs1413 + clhs1414 + clhs1423 + clhs788) + clhs1426 - clhs1429*clhs31 - clhs310*clhs789;
            lhs(9,10)=-clhs1394*clhs800 + clhs1397*clhs803 + clhs1404*clhs341 - clhs1410*clhs332 - clhs1417*clhs1422 - clhs1425*clhs405 + clhs1427*clhs1432*clhs428 + (2.0L/3.0L)*clhs1430 - clhs1431 + clhs310*clhs332;
            lhs(9,11)=clhs53*(clhs1388 + clhs1395*clhs804 - clhs1405*clhs806 + clhs332*clhs805);
            lhs(10,0)=-clhs1173*clhs438 - clhs1393*clhs820 + clhs1433*clhs356 - clhs1434*clhs816 - clhs1435*clhs369 - clhs1436*clhs445 - clhs1437*clhs503 + clhs301*clhs346 - clhs328*clhs810 - clhs603*(clhs328*clhs441 - clhs692*clhs91 + clhs897) + clhs887;
            lhs(10,1)=-clhs116*clhs310 - clhs1402 + (2.0L/3.0L)*clhs1403 - clhs1435*clhs523 + clhs1437*clhs542 - clhs1438*clhs839 - clhs504*clhs903 + clhs898;
            lhs(10,2)=-clhs1398 - 4.0L/3.0L*clhs1399 + clhs1437*clhs555 - clhs1441*clhs843 + clhs1442*clhs547 - clhs310*clhs833 + clhs791 + clhs900;
            lhs(10,3)=clhs53*(clhs103*clhs1440 - clhs136*clhs313 + clhs1381 - clhs1443*clhs557);
            lhs(10,4)=-clhs1173*clhs612 + clhs1203*clhs346 - clhs1205*clhs810 + clhs1315 - clhs1393*clhs864 + clhs1433*clhs591 - clhs1434*clhs861 - clhs1435*clhs602 - clhs1436*clhs616 - clhs1437*clhs647 - clhs603*(clhs1205*clhs441 - clhs1249*clhs91 + clhs1316);
            lhs(10,5)=clhs1317 - clhs1408 + (2.0L/3.0L)*clhs1409 - clhs1435*clhs671 + clhs1437*clhs676 - clhs1438*clhs871 - clhs218*clhs310 - clhs669*clhs903;
            lhs(10,6)=clhs1285 + clhs1318 - clhs1406 - 4.0L/3.0L*clhs1407 + clhs1437*clhs686 - clhs1441*clhs877 + clhs1442*clhs682 - clhs310*clhs875;
            lhs(10,7)=clhs53*(clhs1384 + clhs1440*clhs688 - clhs1443*clhs689 - clhs313*clhs687);
            lhs(10,8)=-clhs1173*clhs738 + clhs1387*clhs346 - clhs1389*clhs810 - clhs1393*clhs896 - clhs1412*(DN(2,1)*clhs441 + clhs283 - clhs286 - clhs294 - clhs311*clhs535 + clhs319*clhs47 + clhs703*clhs822 + clhs894 - clhs895) + clhs1415*clhs1448 - clhs1418*(clhs1439 + clhs1445 - clhs1446 - clhs280 + clhs888) + clhs1433*clhs721 - clhs1434*clhs893 - clhs1435*clhs731 - clhs1436*clhs742 - clhs1437*clhs771 + clhs1444;
            lhs(10,9)=clhs1422*clhs1448 + clhs1424*clhs1432*clhs405 - clhs1428*clhs428 - clhs1430 + (2.0L/3.0L)*clhs1431 - clhs1435*clhs792 + clhs1437*clhs796 - clhs1438*clhs899 - clhs310*clhs313 - clhs790*clhs903;
            lhs(10,10)=clhs1419 - clhs1420 - 4.0L/3.0L*clhs1421 + clhs1422*(-clhs1445 + clhs1446 + clhs1449 + clhs902) - clhs1426*clhs31 + clhs1429 + clhs1437*clhs803 - clhs1441*clhs905 + clhs1442*clhs800 - clhs310*clhs903;
            lhs(10,11)=clhs53*(clhs1390 + clhs1440*clhs805 - clhs1443*clhs806 - clhs313*clhs804);
            lhs(11,0)=clhs1131 - clhs1393*clhs949 - clhs1450*clhs924 - clhs1451*clhs926 + clhs1452*clhs369 + clhs1453*clhs956 - clhs1454*clhs971 - clhs1455*clhs980;
            lhs(11,1)=-clhs1000*clhs1174 - clhs1093*(-clhs1114 + clhs1115 + clhs1116 + clhs1117 + clhs1118 + clhs1119 + clhs1120 - clhs1122 + clhs1123 + clhs1124 + clhs1126) + clhs1159*clhs1328 + clhs1172*clhs1329 + clhs1450*clhs998 + clhs1451*clhs835 + clhs1453*clhs542 + clhs1456*clhs987 + clhs1457*clhs989 + clhs1458*clhs997 - clhs1459*clhs523 + clhs691;
            lhs(11,2)=-clhs1002*clhs1457 - clhs1003*clhs1456 - clhs1007*clhs1458 - clhs1008*clhs1451 + clhs1175 - clhs1450*clhs439 + clhs1452*clhs547 + clhs1453*clhs555;
            lhs(11,3)=clhs1015*clhs1450 + clhs1016*clhs1451 + clhs1186 - clhs1336*clhs776 - clhs1337*clhs774 - clhs1460*clhs508 - clhs1461*clhs513 - clhs1462*clhs954 - clhs1463*clhs557;
            lhs(11,4)=-clhs1048*clhs1450 - clhs1049*clhs1451 - clhs1051*clhs1393 + clhs1053*clhs1453 - clhs1060*clhs1454 - clhs1064*clhs1455 + clhs1370 + clhs1452*clhs602;
            lhs(11,5)=-1.0L/2.0L*clhs1000*clhs102*clhs1280*gamma*tau3 + clhs1091*clhs1451 - clhs1093*(-clhs1355 + clhs1356 + clhs1357 + clhs1358 + clhs1359 + clhs1360 + clhs1361 - clhs1363 + clhs1364 + clhs1365 + clhs1366) + clhs1248 + clhs1328*clhs1371 + clhs1329*clhs1373 + clhs1344*clhs1456 + clhs1345*clhs1457 + clhs1349*clhs1458 + clhs1350*clhs1450 + clhs1453*clhs676 - clhs1459*clhs671;
            lhs(11,6)=-clhs1101*clhs1457 - clhs1103*clhs1456 - clhs1105*clhs1458 - clhs1106*clhs1450 - clhs1107*clhs1451 + clhs1375 + clhs1452*clhs682 + clhs1453*clhs686;
            lhs(11,7)=-clhs1110*clhs1462 + clhs1111*clhs1450 + clhs1112*clhs1451 - clhs1278*clhs1337 - clhs1280*clhs1336 + clhs1376 - clhs1460*clhs659 - clhs1461*clhs663 - clhs1463*clhs689;
            lhs(11,8)=-clhs1132*clhs1450 - clhs1133*clhs1451 - clhs1135*clhs1393 + clhs1137*clhs1453 - clhs1142*clhs1454 - clhs1144*clhs1455 + clhs1385*clhs447 + clhs1412*(clhs1134 - clhs256*clhs922 - clhs260*clhs917 - clhs280*clhs498 + clhs308*clhs950 + clhs330*clhs950 - clhs498*clhs891 - clhs704*clhs951 - clhs724*clhs951) + clhs1452*clhs731 - clhs1464*clhs1468*clhs527 - clhs1464*clhs531*(-clhs1125 + clhs1165 - clhs1465 + clhs1466 - clhs1467 + clhs746) - clhs1469*clhs946;
            lhs(11,9)=(1.0L/2.0L)*N[2]*clhs7*clhs993*(clhs1423 + 3*clhs260 + 6*clhs319 - 6*clhs724) - clhs1000*clhs1427*clhs1472 + clhs1173*clhs1451 + clhs1374*clhs1450*clhs899 + clhs1411 - clhs1422*(clhs1125 + clhs1465 - clhs1466 + clhs1467 + clhs1471 - clhs746) + clhs1453*clhs796 + clhs1456*(clhs1145 - clhs1147 + clhs1150 + clhs1151) + clhs1457*(-clhs1154 - clhs1155 + clhs1156 + clhs1157) + clhs1458*(clhs1166 - clhs1168 + clhs1169 - clhs1170 + clhs1471 - clhs747) - clhs1459*clhs792 + clhs1470*clhs991;
            lhs(11,10)=-1.0L/2.0L*N[2]*clhs1006*clhs7*(clhs1449 + 3*clhs256 + 6*clhs316 - 6*clhs704) - clhs1004*clhs1470 - clhs1179*clhs1457 - clhs1181*clhs1456 - clhs1183*clhs1458 - clhs1184*clhs1450 - clhs1185*clhs1451 + clhs1422*clhs1468 + clhs1424*clhs1472*clhs945 + clhs1444 + clhs1452*clhs800 + clhs1453*clhs803;
            lhs(11,11)=clhs1012*clhs1469 - clhs1187*clhs1462 + clhs1188*clhs1450 + clhs1189*clhs1451 + clhs1386 - clhs1424*clhs1473*clhs428 - clhs1427*clhs1473*clhs405 - clhs1460*clhs780 - clhs1461*clhs784 - clhs1463*clhs806;


}

*/



template<>
void CompressibleNavierStokes<2>::ComputeGaussPointRHSContribution(array_1d<double,12>& rhs, const ElementDataStruct& data,double data_v_sc, double data_k_sc)
{
    const int nodesElement = 3;
    const int SpaceDimension = 2;
    const int nScalarVariables = SpaceDimension + 2;
    const double h = data.h;

    const unsigned int i, j, k, l, m, s, t, tt, p, pp;
    const unsigned int maxi, maxj, maxl, maxl;

    const unsigned int size3 = nScalarVariables * SpaceDimension * nScalarVariables;
    const unsigned int size4 = nScalarVariables * SpaceDimension * nScalarVariables * nScalarVariables;

    const array_1d<double, size3>& A;
    const array_1d<double, size4>& dAdU;

    const array_1d<double,nScalarVariables>&    U_gauss;
    const array_1d<double,nScalarVariables>&    L;
    const array_1d<double,nScalarVariables>&    R;
    const array_1d<double,nNodalVariables*nScalarVariables>&    Lstar;
    const array_1d<double,nScalarVariables*SpaceDimension>&     G;
    const array_1d<double,nScalarVariables,nScalarVariables>&   S;
    const array_1d<double,SpaceDimension,SpaceDimension>&       tau;
    const array_1d<double,SpaceDimension>&                      q;
    const array_1d<double,nScalarVariables*SpaceDimension>&     NN;
    const array_1d<double,nScalarVariables*SpaceDimension>&     gradU;
    const array_1d<double,(nScalarVariables*SpaceDimension)*nNodalVariables>&   gradV;
    const array_1d<double,nScalarVariables>&                    invtauStab;

    const array_1d<double,nNodalVariables>&     Fconv;
    const array_1d<double,nNodalVariables>&     Fstab;
    const array_1d<double,nNodalVariables>&     Fdiff;
    const array_1d<double,nNodalVariables>&     F;
    
	
// Replace with right coefficient according to the time scheme used
    const double& bdf0 = data.bdf0;
    const double& bdf1 = data.bdf1;
    const double& bdf2 = data.bdf2;

    const BoundedMatrix<double,nodesElement,nScalarVariables>& U = data.U;			// Lo mismo de antes // Da tenere come sono e riordinare
    const BoundedMatrix<double,nodesElement,nScalarVariables>& Un = data.Un;
    const BoundedMatrix<double,nodesElement,nScalarVariables>& Unn = data.Unn;

    const BoundedMatrix<double,nodesElement,SpaceDimension>& f_ext = data.f_ext;			
    const array_1d<double,nodesElement>& r = data.r;
    const double mu = data.mu;
    const double nu = data.nu;
    const double lambda = data.lambda;
    const double c_v = data.c_v;
    const double gamma = data.gamma;
    const double cp = c_v*gamma;
    const double v_sc = data_v_sc;
    const double k_sc = data_k_sc;


    const double stab_c1 = 4.0;
    const double stab_c2 = 2.0;    

    // Get shape function values
    const array_1d<double,nnodes>& N = data.N;					// N (in which point?)
    const BoundedMatrix<double,nnodes,dim>& DN = data.DN_DX;	// DN (in which point ?)

    // Auxiliary variables used in the calculation of the RHS
    const array_1d<double,SpaceDim> f_gauss = prod(trans(f_ext), N);      // Da tenere così
//    const BoundedMatrix<double,dim,BlockSize> grad_U = prod(trans(DN), U); /*TO DO: WRONG MIGHT BE REMOVED*/   // Appost
    const array_1d<double,BlockSize> accel_gauss = bdf0*U_gauss+bdf1*prod(trans(Un), N)+bdf2*prod(trans(Unn), N); 	// OK


// Compute the gradient of the variables /dro/dx,dro/dy, dm1/dx dm1/dy dm2/dx dm2/dy de/dx de/dy)

    for (i = 0; i < nScalarVariables; i++){
        U_gauss(i) = 0;
        for (j = 0; j < nodesElement; j++){
            U_gauss(i) += N(j)*U(i + j*nScalarVariables)
        }
    }



    for (i = 0; i < nScalarVariables*SpaceDimension; i++)
        gradU(i) = 0.0;

    for (i = 0; i < nScalarVariables; i++){
        for (k = 0; k < nodesElement; k++){

            gradU(i*SpaceDimension + 0) += DN(k,0)*U(i + k*nScalarVariables);
            gradU(i*SpaceDimension + 1) += DN(k,1)*U(i + k*nScalarVariables);

        }
    }

    const double ro_el = U_gauss(0);
    const double m1_el = U_gauss(1);
    const double m2_el = U_gauss(2);
    const double etot_el = U_gauss(3);

    const double u1_el = m1_el/ro_el;
	const double u2_el = m2_el/ro_el;

	const double norm2u = u1_el*u1_el + u2_el*u2_el;
	const double norm_u = sqrt(norm2u);
	const double norm2m = m1_el*m1_el + m2_el*m2_el;

	const double p_el = (gamma - 1.0)*(etot_el - 0.5 * norm2m/ro_el);
    
    const double SpeedSound = sqrt( gamma * (gamma - 1.0) * (etot_el / ro_el  - 0.5 * norm2u) );


    maxi = nScalarVariables;
	maxj = SpaceDimension;
	maxk = nScalarVariables;
	maxl = 1;

	for (i = 0; i < maxi; i++)
		for (j = 0; j < maxj; j++)
			for (k = 0; k < maxk; k++){

				s = cont(i,j,k,0,maxi,maxj,maxk,maxl);

				A[s] = 0.0;

			}

	maxi = nScalarVariables;
	maxj = SpaceDimension;
	maxk = nScalarVariables;
	maxl = nScalarVariables;

	for (i = 0; i < maxi; i++)
			for (j = 0; j < maxj; j++)
				for (k = 0; k < maxk; k++){
					for (l = 0; l < maxl; l++){

						s = cont(i,j,k,l,maxi,maxj,maxk,maxl);

						dAdU[s] = 0.0;
					}
				}


	const double    dpdro = 0.5*(gamma - 1)*norm2u;
	const double    dpdm1 = -(gamma - 1)*u1_el;
	const double    dpdm2 = -(gamma - 1)*u2_el;
	const double    dpde  =  (gamma - 1);

	const double    d2pdro2 	= -(gamma - 1)*norm2u/ro_el;
	const double    d2pdrodm1 	=  (gamma - 1)*u1_el/ro_el;
	const double    d2pdrodm2 	=  (gamma - 1)*u2_el/ro_el;

	const double    d2pdm12 	= -(gamma - 1)/ro_el;
	const double    d2pdm22 	= -(gamma - 1)/ro_el;

	// Build A

	A[cont(0,0,1,0,4,2,4,1)] = 1.0;
	A[cont(0,1,2,0,4,2,4,1)] = 1.0;

	A[cont(1,0,0,0,4,2,4,1)] = dpdro - u1_el*u1_el;
	A[cont(1,0,1,0,4,2,4,1)] = dpdm1 + 2*u1_el;
	A[cont(1,0,2,0,4,2,4,1)] = dpdm2;
	A[cont(1,0,3,0,4,2,4,1)] = dpde;

	A[cont(1,1,0,0,4,2,4,1)] = -u1_el*u2_el;
	A[cont(1,1,1,0,4,2,4,1)] = u2_el;
	A[cont(1,1,2,0,4,2,4,1)] = u1_el;

	A[cont(2,0,0,0,4,2,4,1)] = -u1_el*u2_el;
	A[cont(2,0,1,0,4,2,4,1)] = u2_el;
	A[cont(2,0,2,0,4,2,4,1)] = u1_el;

	A[cont(2,1,0,0,4,2,4,1)] = dpdro - u2_el*u2_el;
	A[cont(2,1,1,0,4,2,4,1)] = dpdm1;
	A[cont(2,1,2,0,4,2,4,1)] = dpdm2 + 2*u2_el;
	A[cont(2,1,3,0,4,2,4,1)] = dpde;

	A[cont(3,0,0,0,4,2,4,1)] = dpdro*u1_el - (etot_el + p_el)*m1_el/(ro_el*ro_el);
	A[cont(3,0,1,0,4,2,4,1)] = (etot_el + p_el)/ro_el + u1_el * dpdm1;
	A[cont(3,0,2,0,4,2,4,1)] = u1_el * dpdm2;
	A[cont(3,0,3,0,4,2,4,1)] = u1_el * (1 + dpde);

	A[cont(3,1,0,0,4,2,4,1)] = dpdro*u2_el - (etot_el + p_el)*m2_el/(ro_el*ro_el);
	A[cont(3,1,1,0,4,2,4,1)] = u2_el * dpdm1;
	A[cont(3,1,2,0,4,2,4,1)] = (etot_el + p_el)/ro_el + u2_el * dpdm2;
	A[cont(3,1,3,0,4,2,4,1)] = u2_el * (1 + dpde);

//	Build dAdU

	dAdU[cont(1,0,0,0,4,2,4,4)] = d2pdro2 + 2*u1_el*u1_el/ro_el;
	dAdU[cont(1,0,0,1,4,2,4,4)] = d2pdrodm1 - 2*u1_el/ro_el;
	dAdU[cont(1,0,0,2,4,2,4,4)] = d2pdrodm2;

	dAdU[cont(1,0,1,0,4,2,4,4)] = d2pdrodm1 - 2.0 * u1_el/ro_el;
	dAdU[cont(1,0,1,1,4,2,4,4)] = d2pdm12 + 2.0/ro_el;

	dAdU[cont(1,0,2,0,4,2,4,4)] = d2pdrodm2;
	dAdU[cont(1,0,2,2,4,2,4,4)] = d2pdm22;

	/// ------

	dAdU[cont(1,1,0,0,4,2,4,4)] = 2*u1_el*u2_el/ro_el;
	dAdU[cont(1,1,0,1,4,2,4,4)] = -u2_el/ro_el;
	dAdU[cont(1,1,0,2,4,2,4,4)] = -u1_el/ro_el;

	dAdU[cont(1,1,1,0,4,2,4,4)] = -u2_el/ro_el;
	dAdU[cont(1,1,1,2,4,2,4,4)] = 1.0/ro_el;

	dAdU[cont(1,1,2,0,4,2,4,4)] = -u1_el/ro_el;
	dAdU[cont(1,1,2,1,4,2,4,4)] = 1.0/ro_el;

	/// ------

	dAdU[cont(2,0,0,0,4,2,4,4)] = 2*u1_el*u2_el/ro_el;
	dAdU[cont(2,0,0,1,4,2,4,4)] = -u2_el/ro_el;
	dAdU[cont(2,0,0,2,4,2,4,4)] = -u1_el/ro_el;

	dAdU[cont(2,0,1,0,4,2,4,4)] = -u2_el/ro_el;
	dAdU[cont(2,0,1,2,4,2,4,4)] = 1.0/ro_el;

	dAdU[cont(2,0,2,0,4,2,4,4)] = -u1_el/ro_el;
	dAdU[cont(2,0,2,1,4,2,4,4)] = 1.0/ro_el;



	dAdU[cont(2,1,0,0,4,2,4,4)] = d2pdro2 + 2*u2_el*u2_el/ro_el;
	dAdU[cont(2,1,0,1,4,2,4,4)] = d2pdrodm1;
	dAdU[cont(2,1,0,2,4,2,4,4)] = d2pdrodm2 - 2*u2_el/ro_el;

	dAdU[cont(2,1,1,0,4,2,4,4)] = d2pdrodm1;
	dAdU[cont(2,1,1,1,4,2,4,4)] = d2pdm12;

	dAdU[cont(2,1,2,0,4,2,4,4)] = d2pdrodm2 - 2*u2_el/ro_el;
	dAdU[cont(2,1,2,2,4,2,4,4)] = d2pdm22 + 2.0/ro_el;

	/// ------

	dAdU[cont(3,0,0,0,4,2,4,4)] = (m1_el * (2*etot_el + 2*p_el + ro_el*(-2*dpdro + d2pdro2*ro_el)))/(ro_el*ro_el*ro_el);
	dAdU[cont(3,0,0,1,4,2,4,4)] = -(etot_el + dpdm1*m1_el + p_el - dpdro*ro_el - d2pdrodm1*m1_el*ro_el)/(ro_el*ro_el);
	dAdU[cont(3,0,0,2,4,2,4,4)] = (m1_el*(-dpdm2 + d2pdrodm2*ro_el))/(ro_el*ro_el);
	dAdU[cont(3,0,0,3,4,2,4,4)] = -(1 + dpde) * m1_el/(ro_el*ro_el);

	dAdU[cont(3,0,1,0,4,2,4,4)] = -(etot_el + dpdm1*m1_el + p_el - dpdro*ro_el - d2pdrodm1*m1_el*ro_el)/(ro_el*ro_el);
	dAdU[cont(3,0,1,1,4,2,4,4)] = (2*dpdm1 + d2pdm12*m1_el)/ro_el;
	dAdU[cont(3,0,1,2,4,2,4,4)] = dpdm2/ro_el;
	dAdU[cont(3,0,1,3,4,2,4,4)] = (1 + dpde)/ro_el;

	dAdU[cont(3,0,2,0,4,2,4,4)] = (m1_el*(-dpdm2 + d2pdrodm2*ro_el))/(ro_el * ro_el);
	dAdU[cont(3,0,2,1,4,2,4,4)] = dpdm2/ro_el;
	dAdU[cont(3,0,2,2,4,2,4,4)] = (d2pdm22*m1_el)/ro_el;

	dAdU[cont(3,0,3,0,4,2,4,4)] = -(1 + dpde)*m1_el/(ro_el*ro_el);
	dAdU[cont(3,0,3,1,4,2,4,4)] = (1 + dpde)/ro_el;

	dAdU[cont(3,1,0,0,4,2,4,4)] = (m2_el*(2*etot_el + 2*p_el - 2*dpdro*ro_el + d2pdro2*ro_el*ro_el))/(ro_el*ro_el*ro_el);
	dAdU[cont(3,1,0,1,4,2,4,4)] = (m2_el*(-dpdm1 + d2pdrodm1*ro_el))/(ro_el*ro_el);
	dAdU[cont(3,1,0,2,4,2,4,4)] = -(etot_el + p_el - dpdro*ro_el + m2_el*(dpdm2 - d2pdrodm2*ro_el))/(ro_el*ro_el);
	dAdU[cont(3,1,0,3,4,2,4,4)] = (-1 - dpde)*m2_el/(ro_el*ro_el);

	dAdU[cont(3,1,1,0,4,2,4,4)] = (m2_el*(-dpdm1 + d2pdrodm1*ro_el))/(ro_el*ro_el);
	dAdU[cont(3,1,1,1,4,2,4,4)] = (d2pdm12*m2_el)/ro_el;
	dAdU[cont(3,1,1,2,4,2,4,4)] = dpdm1/ro_el;

	dAdU[cont(3,1,2,0,4,2,4,4)] = -(etot_el + p_el - dpdro*ro_el + m2_el*(dpdm2 - d2pdrodm2*ro_el))/(ro_el*ro_el);
	dAdU[cont(3,1,2,1,4,2,4,4)] = dpdm1/ro_el;
	dAdU[cont(3,1,2,2,4,2,4,4)] = (2*dpdm2 + d2pdm22*m2_el)/ro_el;
	dAdU[cont(3,1,2,3,4,2,4,4)] = (1 + dpde)/ro_el;

	dAdU[cont(3,1,3,0,4,2,4,4)] = (-1 - dpde)*m2_el/(ro_el*ro_el);
	dAdU[cont(3,1,3,2,4,2,4,4)] = (1.0 + dpde)/ro_el;










// Stabilization parameters
    
    double tmp = U_gauss(dim+1)/U_gauss(0);
    for(unsigned int ll=0; ll<dim; ll++)
        tmp -=(U_gauss(ll+1)*U_gauss(ll+1))/(2*U_gauss(0)*U_gauss(0));
    double c = sqrt(gamma*(gamma-1)*tmp);

    double tau1inv = 0.0;
    for(unsigned int ll=0; ll<dim; ll++)
        tau1inv += (U_gauss(ll+1)/U_gauss(0))*(U_gauss(ll+1)/U_gauss(0));
    tau1inv = (sqrt(tau1inv)+c)*stab_c2/h;
    double tau2inv = stab_c1*nu/(h*h)+tau1inv;
    double tau3inv = stab_c1*lambda/(U_gauss(0)*cp*h*h)+tau1inv;
        
    const double tau1 = 1/tau1inv;
    const double tau2 = 1/tau2inv;
    const double tau3 = 1/tau3inv;











/* From here, again, terrore morte e devastazione

    const double crhs0 =             DN(0,0)*U(0,1);
const double crhs1 =             DN(1,0)*U(1,1);
const double crhs2 =             DN(2,0)*U(2,1);
const double crhs3 =             crhs0 + crhs1 + crhs2;
const double crhs4 =             DN(0,1)*U(0,2);
const double crhs5 =             DN(1,1)*U(1,2);
const double crhs6 =             DN(2,1)*U(2,2);
const double crhs7 =             crhs4 + crhs5 + crhs6;
const double crhs8 =             crhs3 + crhs7;
const double crhs9 =             N[0]*(U(0,0)*bdf0 + Un(0,0)*bdf1 + Unn(0,0)*bdf2) + N[1]*(U(1,0)*bdf0 + Un(1,0)*bdf1 + Unn(1,0)*bdf2) + N[2]*(U(2,0)*bdf0 + Un(2,0)*bdf1 + Unn(2,0)*bdf2);
const double crhs10 =             N[0]*(U(0,1)*bdf0 + Un(0,1)*bdf1 + Unn(0,1)*bdf2);
const double crhs11 =             2*crhs10;
const double crhs12 =             N[1]*(U(1,1)*bdf0 + Un(1,1)*bdf1 + Unn(1,1)*bdf2);
const double crhs13 =             2*crhs12;
const double crhs14 =             N[2]*(U(2,1)*bdf0 + Un(2,1)*bdf1 + Unn(2,1)*bdf2);
const double crhs15 =             2*crhs14;
const double crhs16 =             2*gamma;
const double crhs17 =             crhs16 - 2;
const double crhs18 =             DN(0,0)*U(0,3);
const double crhs19 =             DN(1,0)*U(1,3);
const double crhs20 =             DN(2,0)*U(2,3);
const double crhs21 =             crhs18 + crhs19 + crhs20;
const double crhs22 =             crhs17*crhs21;
const double crhs23 =             N[0]*f_ext(0,0) + N[1]*f_ext(1,0) + N[2]*f_ext(2,0);
const double crhs24 =             2*N[0]*U(0,0) + 2*N[1]*U(1,0) + 2*N[2]*U(2,0);
const double crhs25 =             crhs23*crhs24;
const double crhs26 =             DN(0,1)*U(0,1);
const double crhs27 =             DN(1,1)*U(1,1);
const double crhs28 =             DN(2,1)*U(2,1);
const double crhs29 =             crhs26 + crhs27 + crhs28;
const double crhs30 =             N[0]*U(0,2) + N[1]*U(1,2) + N[2]*U(2,2);
const double crhs31 =             N[0]*U(0,0) + N[1]*U(1,0) + N[2]*U(2,0);
const double crhs32 =             1.0/crhs31;
const double crhs33 =             2*crhs30*crhs32;
const double crhs34 =             crhs29*crhs33;
const double crhs35 =             N[0]*U(0,1) + N[1]*U(1,1) + N[2]*U(2,1);
const double crhs36 =             2*crhs32*crhs35;
const double crhs37 =             crhs36*crhs7;
const double crhs38 =             crhs16 - 6;
const double crhs39 =             crhs32*crhs35;
const double crhs40 =             crhs3*crhs39;
const double crhs41 =             crhs38*crhs40;
const double crhs42 =             DN(0,0)*U(0,2);
const double crhs43 =             DN(1,0)*U(1,2);
const double crhs44 =             DN(2,0)*U(2,2);
const double crhs45 =             crhs42 + crhs43 + crhs44;
const double crhs46 =             crhs30*crhs32;
const double crhs47 =             crhs45*crhs46;
const double crhs48 =             crhs17*crhs47;
const double crhs49 =             DN(0,1)*U(0,0) + DN(1,1)*U(1,0) + DN(2,1)*U(2,0);
const double crhs50 =             crhs35*crhs49;
const double crhs51 =             pow(crhs31, -2);
const double crhs52 =             2*crhs30*crhs51;
const double crhs53 =             crhs50*crhs52;
const double crhs54 =             DN(0,0)*U(0,0) + DN(1,0)*U(1,0) + DN(2,0)*U(2,0);
const double crhs55 =             crhs51*crhs54;
const double crhs56 =             pow(crhs35, 2);
const double crhs57 =             gamma - 1;
const double crhs58 =             crhs56*crhs57;
const double crhs59 =             pow(crhs30, 2);
const double crhs60 =             crhs57*crhs59;
const double crhs61 =             crhs58 + crhs60;
const double crhs62 =             -2*crhs56 + crhs61;
const double crhs63 =             crhs55*crhs62;
const double crhs64 =             -crhs11 - crhs13 - crhs15 - crhs22 + crhs25 - crhs34 - crhs37 + crhs41 + crhs48 + crhs53 - crhs63;
const double crhs65 =             (1.0L/2.0L)*crhs64*tau2;
const double crhs66 =             N[0]*(U(0,2)*bdf0 + Un(0,2)*bdf1 + Unn(0,2)*bdf2);
const double crhs67 =             2*crhs66;
const double crhs68 =             N[1]*(U(1,2)*bdf0 + Un(1,2)*bdf1 + Unn(1,2)*bdf2);
const double crhs69 =             2*crhs68;
const double crhs70 =             N[2]*(U(2,2)*bdf0 + Un(2,2)*bdf1 + Unn(2,2)*bdf2);
const double crhs71 =             2*crhs70;
const double crhs72 =             DN(0,1)*U(0,3);
const double crhs73 =             DN(1,1)*U(1,3);
const double crhs74 =             DN(2,1)*U(2,3);
const double crhs75 =             crhs72 + crhs73 + crhs74;
const double crhs76 =             crhs17*crhs75;
const double crhs77 =             N[0]*f_ext(0,1) + N[1]*f_ext(1,1) + N[2]*f_ext(2,1);
const double crhs78 =             crhs24*crhs77;
const double crhs79 =             crhs3*crhs33;
const double crhs80 =             crhs36*crhs45;
const double crhs81 =             crhs46*crhs7;
const double crhs82 =             crhs38*crhs81;
const double crhs83 =             crhs29*crhs39;
const double crhs84 =             crhs17*crhs83;
const double crhs85 =             crhs30*crhs54;
const double crhs86 =             2*crhs35*crhs51;
const double crhs87 =             crhs85*crhs86;
const double crhs88 =             crhs49*crhs51;
const double crhs89 =             -2*crhs59 + crhs61;
const double crhs90 =             crhs88*crhs89;
const double crhs91 =             -crhs67 - crhs69 - crhs71 - crhs76 + crhs78 - crhs79 - crhs80 + crhs82 + crhs84 + crhs87 - crhs90;
const double crhs92 =             (1.0L/2.0L)*crhs91*tau2;
const double crhs93 =             N[0]*crhs31;
const double crhs94 =             crhs10 + crhs12 + crhs14;
const double crhs95 =             crhs31*v_sc/mu + 1;
const double crhs96 =             -crhs26 - crhs27 - crhs28 + crhs32*(crhs50 + crhs85) - crhs42 - crhs43 - crhs44;
const double crhs97 =             crhs32*crhs95*crhs96*mu;
const double crhs98 =             crhs32*crhs54;
const double crhs99 =             crhs35*crhs98;
const double crhs100 =             crhs46*crhs49;
const double crhs101 =             -2*crhs0 - 2*crhs1 - crhs100 - 2*crhs2 + crhs7 + 2*crhs99;
const double crhs102 =             (2.0L/3.0L)*crhs101*crhs32*crhs95*mu;
const double crhs103 =             (1.0L/2.0L)*N[0];
const double crhs104 =             crhs22 + crhs34 + crhs37 - crhs41 - crhs48 - crhs53 + crhs63;
const double crhs105 =             (1.0L/2.0L)*tau1*(crhs8 + crhs9);
const double crhs106 =             2*N[0];
const double crhs107 =             -crhs106*crhs23;
const double crhs108 =             DN(0,1)*crhs35;
const double crhs109 =             N[0]*crhs29;
const double crhs110 =             N[0]*crhs7;
const double crhs111 =             gamma - 3;
const double crhs112 =             N[0]*crhs3;
const double crhs113 =             crhs111*crhs112;
const double crhs114 =             N[0]*crhs45;
const double crhs115 =             crhs17*crhs30*crhs51;
const double crhs116 =             crhs114*crhs115;
const double crhs117 =             pow(crhs31, -3);
const double crhs118 =             4*N[0]*crhs117;
const double crhs119 =             crhs30*crhs35*crhs49;
const double crhs120 =             crhs118*crhs119;
const double crhs121 =             crhs51*crhs62;
const double crhs122 =             2*N[0]*crhs117;
const double crhs123 =             crhs54*crhs62;
const double crhs124 =             DN(0,1)*crhs30;
const double crhs125 =             DN(0,0)*crhs35;
const double crhs126 =             N[0]*crhs100;
const double crhs127 =             N[0]*crhs99;
const double crhs128 =             (1.0L/2.0L)*crhs32*crhs64*tau2;
const double crhs129 =             DN(0,0)*crhs30;
const double crhs130 =             crhs114*crhs57;
const double crhs131 =             N[0]*crhs32;
const double crhs132 =             crhs131*crhs50;
const double crhs133 =             crhs131*crhs85;
const double crhs134 =             (1.0L/2.0L)*crhs32*crhs91*tau2;
const double crhs135 =             N[0]*(U(0,3)*bdf0 + Un(0,3)*bdf1 + Unn(0,3)*bdf2);
const double crhs136 =             2*crhs135;
const double crhs137 =             N[1]*(U(1,3)*bdf0 + Un(1,3)*bdf1 + Unn(1,3)*bdf2);
const double crhs138 =             2*crhs137;
const double crhs139 =             N[2]*(U(2,3)*bdf0 + Un(2,3)*bdf1 + Unn(2,3)*bdf2);
const double crhs140 =             2*crhs139;
const double crhs141 =             N[0]*r[0] + N[1]*r[1] + N[2]*r[2];
const double crhs142 =             crhs141*crhs31;
const double crhs143 =             2*crhs142;
const double crhs144 =             crhs23*crhs35;
const double crhs145 =             2*crhs144;
const double crhs146 =             crhs30*crhs77;
const double crhs147 =             2*crhs146;
const double crhs148 =             crhs21*crhs35;
const double crhs149 =             crhs148*crhs16*crhs32;
const double crhs150 =             crhs30*crhs75;
const double crhs151 =             crhs150*crhs16*crhs32;
const double crhs152 =             crhs17*crhs30*crhs35*crhs51;
const double crhs153 =             crhs152*crhs45;
const double crhs154 =             crhs152*crhs29;
const double crhs155 =             crhs32*crhs58;
const double crhs156 =             N[0]*U(0,3);
const double crhs157 =             2*crhs156;
const double crhs158 =             -crhs157;
const double crhs159 =             N[1]*U(1,3);
const double crhs160 =             2*crhs159;
const double crhs161 =             -crhs160;
const double crhs162 =             N[2]*U(2,3);
const double crhs163 =             2*crhs162;
const double crhs164 =             -crhs163;
const double crhs165 =             crhs156 + crhs159 + crhs162;
const double crhs166 =             crhs165*crhs17;
const double crhs167 =             -crhs166;
const double crhs168 =             crhs32*crhs60;
const double crhs169 =             crhs158 + crhs161 + crhs164 + crhs167 + crhs168;
const double crhs170 =             3*crhs155 + crhs169;
const double crhs171 =             crhs170*crhs3*crhs32;
const double crhs172 =             crhs155 + crhs158 + crhs161 + crhs164 + crhs167 + 3*crhs168;
const double crhs173 =             crhs172*crhs32*crhs7;
const double crhs174 =             crhs32*(crhs56 + crhs59);
const double crhs175 =             crhs174*crhs57;
const double crhs176 =             crhs155 + crhs158 + crhs161 + crhs164 + crhs167 + crhs175;
const double crhs177 =             crhs168 + crhs176;
const double crhs178 =             crhs177*crhs35*crhs55;
const double crhs179 =             crhs177*crhs30*crhs88;
const double crhs180 =             (1.0L/2.0L)*crhs57*tau3*(-crhs136 - crhs138 - crhs140 + crhs143 + crhs145 + crhs147 - crhs149 - crhs151 + crhs153 + crhs154 + crhs171 + crhs173 - crhs178 - crhs179);
const double crhs181 =             crhs66 + crhs68 + crhs70;
const double crhs182 =             -crhs0 - crhs1 - 2*crhs100 - crhs2 + 2*crhs4 + 2*crhs5 + 2*crhs6 + crhs99;
const double crhs183 =             (2.0L/3.0L)*crhs182*crhs32*crhs95*mu;
const double crhs184 =             crhs76 + crhs79 + crhs80 - crhs82 - crhs84 - crhs87 + crhs90;
const double crhs185 =             -crhs106*crhs77;
const double crhs186 =             crhs110*crhs111;
const double crhs187 =             crhs17*crhs35*crhs51;
const double crhs188 =             crhs109*crhs187;
const double crhs189 =             crhs30*crhs35*crhs54;
const double crhs190 =             crhs118*crhs189;
const double crhs191 =             crhs51*crhs89;
const double crhs192 =             crhs49*crhs89;
const double crhs193 =             crhs109*crhs57;
const double crhs194 =             crhs135 + crhs137 + crhs139;
const double crhs195 =             crhs142 + crhs144 + crhs146;
const double crhs196 =             3*crhs95*crhs96*mu;
const double crhs197 =             crhs95*mu;
const double crhs198 =             3*lambda*(c_v*crhs31*k_sc/lambda + 1)/c_v;
const double crhs199 =             (1.0L/3.0L)*crhs32*(2*crhs101*crhs197*crhs32*crhs35 + crhs196*crhs46 + crhs198*(crhs165*crhs98 - crhs18 - crhs19 - crhs20 + crhs40 + crhs47 - crhs55*crhs56 - crhs55*crhs59));
const double crhs200 =             (1.0L/3.0L)*crhs32*(-2*crhs182*crhs197*crhs30*crhs32 + crhs196*crhs39 + crhs198*(crhs165*crhs32*crhs49 - crhs56*crhs88 - crhs59*crhs88 - crhs72 - crhs73 - crhs74 + crhs81 + crhs83));
const double crhs201 =             -1.0L/2.0L*crhs170*crhs3 - 1.0L/2.0L*crhs172*crhs7 + (1.0L/2.0L)*crhs177*crhs30*crhs32*crhs49 + (1.0L/2.0L)*crhs177*crhs32*crhs35*crhs54 + crhs21*crhs35*gamma - crhs29*crhs30*crhs32*crhs35*crhs57 - crhs30*crhs32*crhs35*crhs45*crhs57 + crhs30*crhs75*gamma;
const double crhs202 =             (1.0L/4.0L)*tau2*(crhs104 + crhs11 + crhs13 + crhs15 - crhs25);
const double crhs203 =             2*N[0]*crhs32*gamma;
const double crhs204 =             6*gamma - 6;
const double crhs205 =             crhs204*crhs35*crhs51;
const double crhs206 =             crhs170*crhs32;
const double crhs207 =             5*crhs155 + crhs169 + crhs175;
const double crhs208 =             crhs207*crhs51*crhs54;
const double crhs209 =             (1.0L/4.0L)*tau2*(crhs184 + crhs67 + crhs69 + crhs71 - crhs78);
const double crhs210 =             crhs204*crhs30*crhs51;
const double crhs211 =             crhs172*crhs32;
const double crhs212 =             5*crhs168 + crhs176;
const double crhs213 =             crhs212*crhs49*crhs51;
const double crhs214 =             (1.0L/2.0L)*crhs32*gamma*tau3*(crhs136 + crhs138 + crhs140 - crhs143 - crhs145 - crhs147 + crhs149 + crhs151 - crhs153 - crhs154 - crhs171 - crhs173 + crhs178 + crhs179);
const double crhs215 =             2*N[0]*crhs51*gamma;
const double crhs216 =             4*crhs117*crhs30*crhs35;
const double crhs217 =             crhs177*crhs51;
const double crhs218 =             crhs207*crhs51;
const double crhs219 =             crhs212*crhs51;
const double crhs220 =             crhs35*crhs54;
const double crhs221 =             -crhs155 + crhs157 + crhs160 + crhs163 + crhs166 - crhs168 - crhs17*crhs174;
const double crhs222 =             2*N[0]*crhs117*crhs221;
const double crhs223 =             crhs30*crhs49;
const double crhs224 =             N[1]*crhs31;
const double crhs225 =             (1.0L/2.0L)*N[1];
const double crhs226 =             2*N[1];
const double crhs227 =             -crhs226*crhs23;
const double crhs228 =             DN(1,1)*crhs35;
const double crhs229 =             N[1]*crhs29;
const double crhs230 =             N[1]*crhs7;
const double crhs231 =             N[1]*crhs3;
const double crhs232 =             crhs111*crhs231;
const double crhs233 =             N[1]*crhs45;
const double crhs234 =             crhs115*crhs233;
const double crhs235 =             4*N[1]*crhs117;
const double crhs236 =             crhs119*crhs235;
const double crhs237 =             2*N[1]*crhs117;
const double crhs238 =             DN(1,1)*crhs30;
const double crhs239 =             DN(1,0)*crhs35;
const double crhs240 =             N[1]*crhs100;
const double crhs241 =             N[1]*crhs99;
const double crhs242 =             DN(1,0)*crhs30;
const double crhs243 =             crhs233*crhs57;
const double crhs244 =             N[1]*crhs32;
const double crhs245 =             crhs244*crhs50;
const double crhs246 =             crhs244*crhs85;
const double crhs247 =             -crhs226*crhs77;
const double crhs248 =             crhs111*crhs230;
const double crhs249 =             crhs187*crhs229;
const double crhs250 =             crhs189*crhs235;
const double crhs251 =             crhs229*crhs57;
const double crhs252 =             2*N[1]*crhs32*gamma;
const double crhs253 =             2*N[1]*crhs51*gamma;
const double crhs254 =             2*N[1]*crhs117*crhs221;
const double crhs255 =             N[2]*crhs31;
const double crhs256 =             (1.0L/2.0L)*N[2];
const double crhs257 =             2*N[2];
const double crhs258 =             -crhs23*crhs257;
const double crhs259 =             DN(2,1)*crhs35;
const double crhs260 =             N[2]*crhs29;
const double crhs261 =             N[2]*crhs7;
const double crhs262 =             N[2]*crhs3;
const double crhs263 =             crhs111*crhs262;
const double crhs264 =             N[2]*crhs45;
const double crhs265 =             crhs115*crhs264;
const double crhs266 =             4*N[2]*crhs117;
const double crhs267 =             crhs119*crhs266;
const double crhs268 =             2*N[2]*crhs117;
const double crhs269 =             DN(2,1)*crhs30;
const double crhs270 =             DN(2,0)*crhs35;
const double crhs271 =             N[2]*crhs100;
const double crhs272 =             N[2]*crhs99;
const double crhs273 =             DN(2,0)*crhs30;
const double crhs274 =             crhs264*crhs57;
const double crhs275 =             N[2]*crhs32;
const double crhs276 =             crhs275*crhs50;
const double crhs277 =             crhs275*crhs85;
const double crhs278 =             -crhs257*crhs77;
const double crhs279 =             crhs111*crhs261;
const double crhs280 =             crhs187*crhs260;
const double crhs281 =             crhs189*crhs266;
const double crhs282 =             crhs260*crhs57;
const double crhs283 =             2*N[2]*crhs32*gamma;
const double crhs284 =             2*N[2]*crhs51*gamma;
const double crhs285 =             2*N[2]*crhs117*crhs221;
            rhs[0]=-DN(0,0)*crhs65 - DN(0,1)*crhs92 + N[0]*crhs8 + N[0]*crhs9;
            rhs[1]=-DN(0,0)*crhs102 - DN(0,0)*crhs180 - DN(0,1)*crhs97 + N[0]*crhs94 + crhs103*crhs104 - crhs105*(-DN(0,0)*crhs121 + crhs107 + crhs108*crhs52 + crhs109*crhs52 + crhs110*crhs86 - crhs113*crhs86 - crhs116 - crhs120 + crhs122*crhs123) + crhs128*(-crhs110 + crhs111*crhs125 - crhs111*crhs127 + crhs113 - crhs124 + crhs126) + crhs134*(-crhs108 - crhs109 + crhs129*crhs57 + crhs130 + crhs132 - crhs133*crhs57) - crhs23*crhs93;
            rhs[2]=-DN(0,0)*crhs97 - DN(0,1)*crhs180 + DN(0,1)*crhs183 + N[0]*crhs181 + crhs103*crhs184 - crhs105*(-DN(0,1)*crhs191 + crhs112*crhs52 + crhs114*crhs86 + crhs122*crhs192 + crhs125*crhs52 + crhs185 - crhs186*crhs52 - crhs188 - crhs190) - crhs128*(-crhs108*crhs57 + crhs114 + crhs129 + crhs132*crhs57 - crhs133 - crhs193) + crhs134*(crhs111*crhs124 - crhs111*crhs126 - crhs112 - crhs125 + crhs127 + crhs186) - crhs77*crhs93;
            rhs[3]=-DN(0,0)*crhs199 - DN(0,1)*crhs200 + N[0]*crhs194 - N[0]*crhs195 + crhs105*(crhs106*crhs141 + crhs110*crhs219 + crhs112*crhs218 + crhs124*crhs217 + crhs125*crhs217 + crhs130*crhs216 - crhs148*crhs215 - crhs150*crhs215 + crhs193*crhs216 + crhs220*crhs222 + crhs222*crhs223) + crhs131*crhs201 - crhs202*(DN(0,0)*crhs206 - N[0]*crhs208 + crhs107 + crhs108*crhs115 + crhs109*crhs115 + crhs110*crhs187 + crhs112*crhs205 + crhs116 - crhs120*crhs57 - crhs203*crhs21) - crhs209*(DN(0,1)*crhs211 - N[0]*crhs213 + crhs110*crhs210 + crhs112*crhs115 + crhs114*crhs187 + crhs115*crhs125 + crhs185 + crhs188 - crhs190*crhs57 - crhs203*crhs75) + crhs214*(crhs110 + crhs112 + crhs124 + crhs125 - crhs126 - crhs127);
            rhs[4]=-DN(1,0)*crhs65 - DN(1,1)*crhs92 + N[1]*crhs8 + N[1]*crhs9;
            rhs[5]=-DN(1,0)*crhs102 - DN(1,0)*crhs180 - DN(1,1)*crhs97 + N[1]*crhs94 + crhs104*crhs225 - crhs105*(-DN(1,0)*crhs121 + crhs123*crhs237 + crhs227 + crhs228*crhs52 + crhs229*crhs52 + crhs230*crhs86 - crhs232*crhs86 - crhs234 - crhs236) + crhs128*(crhs111*crhs239 - crhs111*crhs241 - crhs230 + crhs232 - crhs238 + crhs240) + crhs134*(-crhs228 - crhs229 + crhs242*crhs57 + crhs243 + crhs245 - crhs246*crhs57) - crhs224*crhs23;
            rhs[6]=-DN(1,0)*crhs97 - DN(1,1)*crhs180 + DN(1,1)*crhs183 + N[1]*crhs181 - crhs105*(-DN(1,1)*crhs191 + crhs192*crhs237 + crhs231*crhs52 + crhs233*crhs86 + crhs239*crhs52 + crhs247 - crhs248*crhs52 - crhs249 - crhs250) - crhs128*(-crhs228*crhs57 + crhs233 + crhs242 + crhs245*crhs57 - crhs246 - crhs251) + crhs134*(crhs111*crhs238 - crhs111*crhs240 - crhs231 - crhs239 + crhs241 + crhs248) + crhs184*crhs225 - crhs224*crhs77;
            rhs[7]=-DN(1,0)*crhs199 - DN(1,1)*crhs200 + N[1]*crhs194 - N[1]*crhs195 + crhs105*(crhs141*crhs226 - crhs148*crhs253 - crhs150*crhs253 + crhs216*crhs243 + crhs216*crhs251 + crhs217*crhs238 + crhs217*crhs239 + crhs218*crhs231 + crhs219*crhs230 + crhs220*crhs254 + crhs223*crhs254) + crhs201*crhs244 - crhs202*(DN(1,0)*crhs206 - N[1]*crhs208 + crhs115*crhs228 + crhs115*crhs229 + crhs187*crhs230 + crhs205*crhs231 - crhs21*crhs252 + crhs227 + crhs234 - crhs236*crhs57) - crhs209*(DN(1,1)*crhs211 - N[1]*crhs213 + crhs115*crhs231 + crhs115*crhs239 + crhs187*crhs233 + crhs210*crhs230 + crhs247 + crhs249 - crhs250*crhs57 - crhs252*crhs75) + crhs214*(crhs230 + crhs231 + crhs238 + crhs239 - crhs240 - crhs241);
            rhs[8]=-DN(2,0)*crhs65 - DN(2,1)*crhs92 + N[2]*crhs8 + N[2]*crhs9;
            rhs[9]=-DN(2,0)*crhs102 - DN(2,0)*crhs180 - DN(2,1)*crhs97 + N[2]*crhs94 + crhs104*crhs256 - crhs105*(-DN(2,0)*crhs121 + crhs123*crhs268 + crhs258 + crhs259*crhs52 + crhs260*crhs52 + crhs261*crhs86 - crhs263*crhs86 - crhs265 - crhs267) + crhs128*(crhs111*crhs270 - crhs111*crhs272 - crhs261 + crhs263 - crhs269 + crhs271) + crhs134*(-crhs259 - crhs260 + crhs273*crhs57 + crhs274 + crhs276 - crhs277*crhs57) - crhs23*crhs255;
            rhs[10]=-DN(2,0)*crhs97 - DN(2,1)*crhs180 + DN(2,1)*crhs183 + N[2]*crhs181 - crhs105*(-DN(2,1)*crhs191 + crhs192*crhs268 + crhs262*crhs52 + crhs264*crhs86 + crhs270*crhs52 + crhs278 - crhs279*crhs52 - crhs280 - crhs281) - crhs128*(-crhs259*crhs57 + crhs264 + crhs273 + crhs276*crhs57 - crhs277 - crhs282) + crhs134*(crhs111*crhs269 - crhs111*crhs271 - crhs262 - crhs270 + crhs272 + crhs279) + crhs184*crhs256 - crhs255*crhs77;
            rhs[11]=-DN(2,0)*crhs199 - DN(2,1)*crhs200 + N[2]*crhs194 - N[2]*crhs195 + crhs105*(crhs141*crhs257 - crhs148*crhs284 - crhs150*crhs284 + crhs216*crhs274 + crhs216*crhs282 + crhs217*crhs269 + crhs217*crhs270 + crhs218*crhs262 + crhs219*crhs261 + crhs220*crhs285 + crhs223*crhs285) + crhs201*crhs275 - crhs202*(DN(2,0)*crhs206 - N[2]*crhs208 + crhs115*crhs259 + crhs115*crhs260 + crhs187*crhs261 + crhs205*crhs262 - crhs21*crhs283 + crhs258 + crhs265 - crhs267*crhs57) - crhs209*(DN(2,1)*crhs211 - N[2]*crhs213 + crhs115*crhs262 + crhs115*crhs270 + crhs187*crhs264 + crhs210*crhs261 + crhs278 + crhs280 - crhs281*crhs57 - crhs283*crhs75) + crhs214*(crhs261 + crhs262 + crhs269 + crhs270 - crhs271 - crhs272);
*/


}


template<>
double CompressibleNavierStokes<2>::ShockCapturingViscosity(const ElementDataStruct& data)
{
    const int nnodes = 3;
    const int dim = 2;
    const int BlockSize = dim+2;

   const double h = data.h;                                // Characteristic element size
   const double alpha = 0.8;                               // Algorithm constant
   const double tol = 0.001;                               

    const double& bdf0 = data.bdf0;
    const double& bdf1 = data.bdf1;
    const double& bdf2 = data.bdf2;

    const BoundedMatrix<double,nnodes,BlockSize>& U = data.U;
    const BoundedMatrix<double,nnodes,BlockSize>& Un = data.Un;
    const BoundedMatrix<double,nnodes,BlockSize>& Unn = data.Unn;
    const BoundedMatrix<double,nnodes,dim>& f_ext = data.f_ext;
    const double gamma = data.gamma;
    double v_sc = 0.0;                                      //Shock capturing viscosity
    BoundedMatrix<double,dim,1> res_m;
    res_m(0,0) =0; res_m(1,0) =0;
    
    // Get shape function values
    const array_1d<double,nnodes>& N = data.N;
    const BoundedMatrix<double,nnodes,dim>& DN = data.DN_DX;

    // Auxiliary variables used in the calculation of the RHS
    const array_1d<double,BlockSize> U_gauss = prod(trans(U), N);
    const array_1d<double,dim> f_gauss = prod(trans(f_ext), N);
    const BoundedMatrix<double,BlockSize,dim> grad_U = prod(trans(U), DN);     // Dfi/Dxj
    const array_1d<double,BlockSize> accel_gauss = bdf0*U_gauss+bdf1*prod(trans(Un), N)+bdf2*prod(trans(Unn), N);
   
    const double cres_m0 =             gamma - 1;
const double cres_m1 =             N[0]*U(0,0) + N[1]*U(1,0) + N[2]*U(2,0);
const double cres_m2 =             DN(0,1)*U(0,1) + DN(1,1)*U(1,1) + DN(2,1)*U(2,1);
const double cres_m3 =             N[0]*U(0,2) + N[1]*U(1,2) + N[2]*U(2,2);
const double cres_m4 =             1.0/cres_m1;
const double cres_m5 =             cres_m3*cres_m4;
const double cres_m6 =             DN(0,1)*U(0,2) + DN(1,1)*U(1,2) + DN(2,1)*U(2,2);
const double cres_m7 =             N[0]*U(0,1) + N[1]*U(1,1) + N[2]*U(2,1);
const double cres_m8 =             cres_m4*cres_m7;
const double cres_m9 =             gamma - 3;
const double cres_m10 =             DN(0,0)*U(0,1) + DN(1,0)*U(1,1) + DN(2,0)*U(2,1);
const double cres_m11 =             DN(0,0)*U(0,2) + DN(1,0)*U(1,2) + DN(2,0)*U(2,2);
const double cres_m12 =             DN(0,1)*U(0,0) + DN(1,1)*U(1,0) + DN(2,1)*U(2,0);
const double cres_m13 =             pow(cres_m1, -2);
const double cres_m14 =             cres_m13*cres_m3*cres_m7;
const double cres_m15 =             (1.0L/2.0L)*cres_m13;
const double cres_m16 =             DN(0,0)*U(0,0) + DN(1,0)*U(1,0) + DN(2,0)*U(2,0);
const double cres_m17 =             pow(cres_m7, 2);
const double cres_m18 =             pow(cres_m3, 2);
const double cres_m19 =             cres_m0*cres_m17 + cres_m0*cres_m18;
            res_m(0,0)=-N[0]*(U(0,1)*bdf0 + Un(0,1)*bdf1 + Unn(0,1)*bdf2) - N[1]*(U(1,1)*bdf0 + Un(1,1)*bdf1 + Unn(1,1)*bdf2) - N[2]*(U(2,1)*bdf0 + Un(2,1)*bdf1 + Unn(2,1)*bdf2) + cres_m0*cres_m11*cres_m5 - cres_m0*(DN(0,0)*U(0,3) + DN(1,0)*U(1,3) + DN(2,0)*U(2,3)) + cres_m1*(N[0]*f_ext(0,0) + N[1]*f_ext(1,0) + N[2]*f_ext(2,0)) + cres_m10*cres_m8*cres_m9 + cres_m12*cres_m14 - cres_m15*cres_m16*(-2*cres_m17 + cres_m19) - cres_m2*cres_m5 - cres_m6*cres_m8;
            res_m(1,0)=-N[0]*(U(0,2)*bdf0 + Un(0,2)*bdf1 + Unn(0,2)*bdf2) - N[1]*(U(1,2)*bdf0 + Un(1,2)*bdf1 + Unn(1,2)*bdf2) - N[2]*(U(2,2)*bdf0 + Un(2,2)*bdf1 + Unn(2,2)*bdf2) + cres_m0*cres_m2*cres_m8 - cres_m0*(DN(0,1)*U(0,3) + DN(1,1)*U(1,3) + DN(2,1)*U(2,3)) + cres_m1*(N[0]*f_ext(0,1) + N[1]*f_ext(1,1) + N[2]*f_ext(2,1)) - cres_m10*cres_m5 - cres_m11*cres_m8 - cres_m12*cres_m15*(-2*cres_m18 + cres_m19) + cres_m14*cres_m16 + cres_m5*cres_m6*cres_m9;


    double norm_res_m;
    norm_res_m = sqrt(res_m(0,0)*res_m(0,0)+res_m(1,0)*res_m(1,0));

    double norm_gradm = 0.0;                                    // Frobenius norm of momentum gradient
    for (unsigned int i=1; i<dim+1; i++){
        for (unsigned int j=0; j<dim; j++)
            norm_gradm += grad_U(i,j)*grad_U(i,j);
    }
    norm_gradm = sqrt(norm_gradm);
    
    if (norm_gradm>tol)
        v_sc = 0.5*h*alpha*(norm_res_m/norm_gradm);
    
    return v_sc;
    
}


template<>
double CompressibleNavierStokes<2>::ShockCapturingConductivity(const ElementDataStruct& data)
{
    const int nnodes = 3;
    const int dim = 2;
    const int BlockSize = dim+2;

   const double h = data.h;                                // Characteristic element size (which?)
   const double alpha = 0.8;                               // Algorithm constant
   const double tol = 0.001;                               

    const double& bdf0 = data.bdf0;
    const double& bdf1 = data.bdf1;
    const double& bdf2 = data.bdf2;

    const BoundedMatrix<double,nnodes,BlockSize>& U = data.U;
    const BoundedMatrix<double,nnodes,BlockSize>& Un = data.Un;
    const BoundedMatrix<double,nnodes,BlockSize>& Unn = data.Unn;
    const BoundedMatrix<double,nnodes,dim>& f_ext = data.f_ext;
    const array_1d<double,nnodes>& r = data.r;
    const double gamma = data.gamma;
    double k_sc = 0.0;          // Shock Capturing Conductivity
    BoundedMatrix<double,dim,1> res_e;
    res_e(0,0) =0;

    // Get shape function values
    const array_1d<double,nnodes>& N = data.N;
    const BoundedMatrix<double,nnodes,dim>& DN = data.DN_DX;

    // Auxiliary variables used in the calculation of the RHS
    const array_1d<double,BlockSize> U_gauss = prod(trans(U), N);
    const array_1d<double,dim> f_gauss = prod(trans(f_ext), N);
    const BoundedMatrix<double,BlockSize,dim> grad_U = prod(trans(U), DN);     // Dfi/Dxj
    const array_1d<double,BlockSize> accel_gauss = bdf0*U_gauss+bdf1*prod(trans(Un), N)+bdf2*prod(trans(Unn), N);
    
    const double cres_e0 =             N[0]*U(0,0) + N[1]*U(1,0) + N[2]*U(2,0);
const double cres_e1 =             N[0]*U(0,1) + N[1]*U(1,1) + N[2]*U(2,1);
const double cres_e2 =             N[0]*U(0,2) + N[1]*U(1,2) + N[2]*U(2,2);
const double cres_e3 =             1.0/cres_e0;
const double cres_e4 =             cres_e3*gamma;
const double cres_e5 =             gamma - 1;
const double cres_e6 =             pow(cres_e0, -2);
const double cres_e7 =             cres_e1*cres_e2*cres_e5*cres_e6;
const double cres_e8 =             (1.0L/2.0L)*cres_e3;
const double cres_e9 =             N[0]*U(0,3);
const double cres_e10 =             -2*cres_e9;
const double cres_e11 =             N[1]*U(1,3);
const double cres_e12 =             -2*cres_e11;
const double cres_e13 =             N[2]*U(2,3);
const double cres_e14 =             -2*cres_e13;
const double cres_e15 =             -2*cres_e5*(cres_e11 + cres_e13 + cres_e9);
const double cres_e16 =             pow(cres_e2, 2);
const double cres_e17 =             cres_e3*cres_e5;
const double cres_e18 =             cres_e16*cres_e17;
const double cres_e19 =             pow(cres_e1, 2);
const double cres_e20 =             cres_e17*cres_e19;
const double cres_e21 =             cres_e10 + cres_e12 + cres_e14 + cres_e15 + cres_e20;
const double cres_e22 =             (1.0L/2.0L)*cres_e6*(cres_e17*(cres_e16 + cres_e19) + cres_e18 + cres_e21);
            res_e(0,0)=-N[0]*(U(0,3)*bdf0 + Un(0,3)*bdf1 + Unn(0,3)*bdf2) - N[1]*(U(1,3)*bdf0 + Un(1,3)*bdf1 + Unn(1,3)*bdf2) - N[2]*(U(2,3)*bdf0 + Un(2,3)*bdf1 + Unn(2,3)*bdf2) + cres_e0*(N[0]*r[0] + N[1]*r[1] + N[2]*r[2]) - cres_e1*cres_e22*(DN(0,0)*U(0,0) + DN(1,0)*U(1,0) + DN(2,0)*U(2,0)) - cres_e1*cres_e4*(DN(0,0)*U(0,3) + DN(1,0)*U(1,3) + DN(2,0)*U(2,3)) + cres_e1*(N[0]*f_ext(0,0) + N[1]*f_ext(1,0) + N[2]*f_ext(2,0)) - cres_e2*cres_e22*(DN(0,1)*U(0,0) + DN(1,1)*U(1,0) + DN(2,1)*U(2,0)) - cres_e2*cres_e4*(DN(0,1)*U(0,3) + DN(1,1)*U(1,3) + DN(2,1)*U(2,3)) + cres_e2*(N[0]*f_ext(0,1) + N[1]*f_ext(1,1) + N[2]*f_ext(2,1)) + cres_e7*(DN(0,0)*U(0,2) + DN(1,0)*U(1,2) + DN(2,0)*U(2,2)) + cres_e7*(DN(0,1)*U(0,1) + DN(1,1)*U(1,1) + DN(2,1)*U(2,1)) + cres_e8*(3*cres_e18 + cres_e21)*(DN(0,1)*U(0,2) + DN(1,1)*U(1,2) + DN(2,1)*U(2,2)) + cres_e8*(DN(0,0)*U(0,1) + DN(1,0)*U(1,1) + DN(2,0)*U(2,1))*(cres_e10 + cres_e12 + cres_e14 + cres_e15 + cres_e18 + 3*cres_e20);


    double norm_res_e;
    norm_res_e = sqrt(res_e(0,0)*res_e(0,0));

    double norm_grade = 0.0;              // Frobenius norm of total energy gradient
    for (unsigned int i=0; i<dim; i++)
        norm_grade += grad_U(dim+1,i)*grad_U(dim+1,i);
    norm_grade = sqrt(norm_grade);
    
 
    if (norm_grade>tol)
        k_sc = 0.5*h*alpha*(norm_res_e/norm_grade);

    return k_sc;

    }


}