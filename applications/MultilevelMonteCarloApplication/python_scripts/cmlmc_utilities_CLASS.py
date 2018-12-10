from __future__ import absolute_import, division # makes KratosMultiphysics backward compatible with python 2.6 and 2.7
import numpy as np
import KratosMultiphysics
import time

# Import exaqute
# from exaqute.ExaquteTaskPyCOMPSs import *   # to exequte with pycompss
# from exaqute.ExaquteTaskHyperLoom import *  # to exequte with the IT4 scheduler
# from exaqute.ExaquteTaskLocal import *      # to execute with python3
# get_value_from_remote is the equivalent of compss_wait_on
# in the future, when everything is integrated with the it4i team, putting exaqute.ExaquteTaskHyperLoom you can launch your code with their scheduler instead of BSC


'''
This utility contains all the functions to perform the Continuation Multilevel Monte Carlo (CMLMC) algorithm described in [PNL17]

References:
[PNL17] M. Pisaroni; F. Nobile; P. Leyland : A Continuation Multi Level Monte Carlo (C-MLMC) method for uncertainty quantification in compressible inviscid aerodynamics; Computer Methods in Applied Mechanics and Engineering, vol 326, pp 20-50, 2017. DOI : 10.1016/j.cma.2017.07.030.
'''

class StatisticalVariable(object):
    '''The base class for the quantity of interest and other statistical variables computed'''
    def __init__(self):
        '''constructor of the class
        Keyword arguments:
        self : an instance of a class
        '''

        '''values of the variable, divided per level'''
        self.values = []
        '''mean of the variable per each level'''
        self.mean = []
        '''sample variance of the variable per each level'''
        self.sample_variance = []
        '''second moment of the variable per each level'''
        self.second_moment = []
        '''bias error of the variable'''
        self.bias_error = None
        '''statistical error of the variable'''
        self.statistical_error = None
        '''type of variable: scalar or field'''
        self.type = None


    '''
    function updating mean and second moment values and computing the sample variance
    M_{2,n} = sum_{i=1}^{n} (x_i - mean(x)_n)^2
    M_{2,n} = M_{2,n-1} + (x_n - mean(x)_{n-1}) * (x_n - mean(x)_{n})
    s_n^2 = M_{2,n} / (n-1)
    '''
    def UpdateOnepassMeanVariance(self,level,i_sample):
        sample = self.values[level][i_sample]
        old_mean = self.mean[level]
        old_M2 = self.second_moment[level]
        nsamples = i_sample + 1
        delta = np.subtract(sample, old_mean)
        if nsamples == 1:
            new_mean = sample
            new_M2 = np.zeros(np.size(sample))
            new_M2 = np.asscalar(new_M2) # do so to have a list of scalars, and not a list of arrays of one element
            new_sample_variance = np.zeros(np.size(sample))
            new_sample_variance = np.asscalar(new_sample_variance) # do so to have a list of scalars, and not a list of arrays of one element
        else:
            new_mean = old_mean + np.divide(delta,nsamples)
            new_M2 = old_M2 + delta*np.subtract(sample,new_mean)
            new_sample_variance = np.divide(new_M2,np.subtract(nsamples,1))
        self.mean[level] = new_mean
        self.second_moment[level] = new_M2
        self.sample_variance[level] = new_sample_variance
        del(new_mean, new_M2, new_sample_variance)


'''TODO: inser distinction between scalar and field Quantity of Interests'''
class MultilevelMonteCarlo(object):
    '''The base class for the MultilevelMonteCarlo-classes'''
    def __init__(self,settings):
        '''constructor of the MultilevelMonteCarlo-Object
        Keyword arguments:
        self     : an instance of a class
        settings : the settings of the Multilevel Monte Carlo simulation
        '''

        '''
        k0   : Certainty Parameter 0 rates
        k1   : Certainty Parameter 1 rates
        r1   : Cost increase first iterations C-MLMC
        r2   : Cost increase final iterations C-MLMC
        tol0 : Tolerance iter 0
        tolF : Tolerance final
        cphi : Confidence on tolerance
        N0   : Number of samples for iter 0
        L0   : Number of levels for iter 0
        Lmax : Maximum number of levels
        mesh_refinement_coefficient : coefficient of mesh refinement
        initial_mesh_size : size of first mesh considered
        minimum_add_level : minimum number of samples to add if at least one is going to be added
        '''
        self.settings = {"k0":settings[0],\
                         "k1":settings[1],\
                         "r1":settings[2],\
                         "r2":settings[3],\
                         "tol0":settings[4],\
                         "tolF":settings[5],\
                         "cphi":settings[6],\
                         "N0":settings[7],\
                         "Lscreening":settings[8],\
                         "Lmax":settings[9],\
                         "mesh_refinement_coefficient":settings[10],\
                         "initial_mesh_size":settings[11]}
        self.settings["minimum_add_level"] = 6.
        '''current_number_levels : number of levels of current iteration'''
        self.current_number_levels = self.settings["Lscreening"]
        '''previous_number_levels : number of levels of previous iteration'''
        self.previous_number_levels = None
        '''number_samples : total number of samples at current iteration'''        
        self.number_samples = [self.settings["N0"] for i in range (self.settings["Lscreening"]+1)]
        '''difference_number_samples : difference between number of samples of current and previous iterations'''
        self.difference_number_samples = None
        '''previous_number_samples : total number of samples of previous iteration'''
        self.previous_number_samples = None
        '''rates_error : dictionary containing the values of the parameters
        calpha : coefficient of the function maximizing bias
        alpha  : exponent of the function maximizing bias
        cbeta  : coefficient of the function maximizing statistical error
        beta   : exponent of the function maximizing statistical error
        cgamma : coefficient of the function maximizing cost
        gamma  : exponent of the function maximizing cost
        '''
        self.rates_error = {"calpha":None, "alpha":None, "cbeta":None, "beta":None, "cgamma":None, "gamma":None}
        '''mesh_parameters : reciprocal of minimal mesh size'''        
        self.mesh_parameters = []
        '''size_mesh : minimal mesh size'''
        self.sizes_mesh = []
        '''BayesianVariance : Bayesian variance'''
        self.BayesianVariance = []
        '''number_iterations : theoretical number of iterations the MLMC algorithm will perform'''
        self.number_iterations_iE = None
        '''convergence : boolean variable defining if MLMC algorithm is convergenced'''
        self.convergence = False
        '''current_iteration : current iteration of MLMC algorithm'''
        self.current_iteration = None
        '''tolerance_i : tolerance of i^th-iteration considered in MLMC algorithm'''
        self.tolerance_i = None
        '''theta_i : splitting parameter \in(0,1) that affects bias and statistical error in the computation of the total error'''
        self.theta_i = None
        '''mean_mlmc_QoI : MLMC estimator for the mean value of the Quantity of Interest'''
        self.mean_mlmc_QoI = None
        '''TErr : total error of MLMC algorithm, the sum of bias and statistical error is an overstmation of the real total error
                  TErr := \abs(E^MLMC[QoI] - E[QoI])'''
        self.TErr = None
        '''compute mesh parameter for each mesh'''
        self.ComputeMeshParameters()

        '''difference_QoI : Quantity of Interest of the considered problem organized in consecutive levels
                            difference_QoI.values := Y_l = QoI_M_l - Q_M_l-1'''
        self.difference_QoI = StatisticalVariable()
        self.difference_QoI.values = [[] for i in range (self.settings["Lscreening"]+1)] # list containing Y_{l}^{i} = Q_{m_l} - Q_{m_{l-1}}
        '''time_ML : time to perform a single MLMC simulation (i.e. one value of difference_QoI.values) organized in consecutive levels'''
        self.difference_QoI.type = "scalar"
        self.time_ML = StatisticalVariable()
        self.time_ML.values = [[] for i in range (self.settings["Lscreening"]+1)] # list containing the time to compute the level=l simulations

        '''########################################################################
        # observation: levels start from level 0                                  #
        #              length arrays and lists starts from 1                      #
        # then we have a difference of 1 between length lists and levels          #
        # e.g. self.current_level = len(self.number_samples) - 1                  #
        #      or                                                                 #
        #      self.current_level = len(self.difference_QoI.difference_value) - 1 #
        ########################################################################'''


    '''
    function finalizing the screening phase of the MLMC algorithm
    Usage: It is designed to be called ONCE, AFTER the screening phase
    '''
    def FinalizeScreeningPhase(self):
        '''prepare lists'''
        self.difference_QoI.mean = [[] for i in range (self.settings["Lscreening"]+1)]
        self.difference_QoI.sample_variance = [[] for i in range (self.settings["Lscreening"]+1)]
        self.difference_QoI.second_moment = [[] for i in range (self.settings["Lscreening"]+1)]
        self.time_ML.mean = [[] for i in range (self.settings["Lscreening"]+1)]
        self.time_ML.sample_variance = [[] for i in range (self.settings["Lscreening"]+1)]
        self.time_ML.second_moment = [[] for i in range (self.settings["Lscreening"]+1)]
        '''compute mean, sample variance and second moment for difference QoI and time ML'''
        for level in range (self.current_number_levels+1):
            for i_sample in range(self.number_samples[level]):
                self.difference_QoI.UpdateOnepassMeanVariance(level,i_sample)
                self.time_ML.UpdateOnepassMeanVariance(level,i_sample)
        '''compute parameters by least square fit to estimate Bayesian VAR'''
        self.ComputeRatesLS()
        '''compute Bayesian VAR V^c[Y_l]'''
        self.EstimateBayesianVariance(self.current_number_levels)
        '''compute i_E, number of iterations'''
        self.ComputeNumberIterationsMLMC()
        '''start first iteration, we are entering in the MLMC algorithm'''
        self.current_iteration = 1
    
    '''
    function performing all the required operations that should be executed
    (for each step) BEFORE the MLMC solution step
    '''
    def InitializeMLMCPhase(self):
        '''compute tolerance for the i^th iteration'''
        self.ComputeTolerancei()
        '''Compute Optimal Number of Levels for iteration i L_i'''
        self.ComputeLevels()
        '''compute theta splitting parameter according to the current_number_levels and tolerance_i'''
        self.ComputeTheta(self.current_number_levels)
        '''compute number of samples according to BayesianVariance and theta_i parameters'''
        self.ComputeNumberSamples()
        '''prepare lists'''
        for i in range (self.current_number_levels - self.previous_number_levels): # append a list for the new level
            self.difference_QoI.values.append([])
            self.time_ML.values.append([])


    '''function performing all the required operations that should be executed
    (for each step) AFTER the MLMC solution step'''
    def FinalizeMLMCPhase(self):
        '''prepare lists'''
        for i in range (self.current_number_levels - self.previous_number_levels): # append a list for the new level
            self.difference_QoI.mean.append([])
            self.difference_QoI.sample_variance.append([])
            self.difference_QoI.second_moment.append([])
            self.time_ML.mean.append([])
            self.time_ML.sample_variance.append([])
            self.time_ML.second_moment.append([])
        '''compute mean, second moment and sample variance'''
        for level in range (self.current_number_levels+1):
            # for i_sample in range(self.difference_number_samples[level]):
            for i_sample in range(self.previous_number_samples[level],self.number_samples[level]):
                self.difference_QoI.UpdateOnepassMeanVariance(level,i_sample)
                self.time_ML.UpdateOnepassMeanVariance(level,i_sample)
        '''compute estimatior MLMC mean QoI'''
        self.compute_mean_mlmc_QoI()
        '''compute parameters by least square fit'''
        self.ComputeRatesLS()
        '''compute Bayesian variance'''
        self.EstimateBayesianVariance(self.current_number_levels)
        '''compute total error of the MLMC simulation'''
        self.ComputeTotalErrorMLMC()
        '''update number of levels'''
        self.previous_number_levels = self.current_number_levels
        '''convergence reached if: i) current_iteration >= number_iterations_iE
                                  ii) TErr < tolerance_i
           if not update current_iteration'''
        if (self.current_iteration >= self.number_iterations_iE) and (self.TErr < self.tolerance_i):
            self.convergence = True
        else:
            self.current_iteration = self.current_iteration + 1


    '''
    function printing informations about screening phase
    '''
    def ScreeningInfoScreeningPhase(self):
        # print("values computed of QoI = ",self.difference_QoI.values)
        # print("values computed time_ML",self.time_ML.values)
        print("mean and variance difference_QoI = ",self.difference_QoI.mean,self.difference_QoI.sample_variance)
        print("mean and variance time_ML",self.time_ML.mean,self.time_ML.sample_variance)
        print("rates coefficient = ",self.rates_error)
        print("estimated Bayesian variance = ",self.BayesianVariance)
        print("minimum number of MLMC iterations = ",self.number_iterations_iE)


    '''
    function printing informations about initializing MLMC phase
    '''
    def ScreeningInfoInitializeMLMCPhase(self):
        print("\n ######## CMLMC iter = ",self.current_iteration,"######## \n")
        print("current tolerance = ",self.tolerance_i)
        print("updated estimated Bayesian Variance initialize phase = ",self.BayesianVariance)
        print("current number of levels = ",self.current_number_levels)
        print("previous number of levels = ",self.previous_number_levels)
        print("current splitting parameter = ",self.theta_i)
        print("current number of samples",self.number_samples)
        print("difference number of samples = ",self.difference_number_samples)
        print("previous number of samples = ",self.previous_number_samples)


    '''
    function printing informations about finalizing MLMC phase
    '''
    def ScreeningInfoFinalizeMLMCPhase(self):
        # print("values computed of QoI = ",self.difference_QoI.values)
        # print("values computed time_ML",self.time_ML.values)
        print("mean and variance difference_QoI = ",self.difference_QoI.mean,self.difference_QoI.sample_variance)
        print("mean and variance time_ML",self.time_ML.mean,self.time_ML.sample_variance)
        print("rates coefficient = ",self.rates_error)
        print("estimated Bayesian variance = ",self.BayesianVariance)
        print("multilevel monte carlo mean estimator = ",self.mean_mlmc_QoI)
        print("TErr = bias + statistical error = ",self.TErr)
        

    '''
    function adding the QoI and MLMC time values to the corresponding level and object of the variable
    '''
    def AddResults(self,simulation_results):
        difference_QoI_value = simulation_results["QoI_finer_level"] - simulation_results["QoI_coarser_level"]
        self.difference_QoI.values[simulation_results["finer_level"]] = np.append(self.difference_QoI.values[simulation_results["finer_level"]],difference_QoI_value)
        self.time_ML.values[simulation_results["finer_level"]] = np.append(self.time_ML.values[simulation_results["finer_level"]],simulation_results["total_MLMC_time"])


    '''
    function giving as output the mesh discretization parameter
    the mesh parameter is the reciprocal of the minimum mesh size of the grid
    h_lev=h_0*M^(-lev)
    '''
    def ComputeMeshParameters(self):
        h0 = self.settings["initial_mesh_size"]
        M  = self.settings["mesh_refinement_coefficient"]
        for level in range(self.settings["Lmax"]+1):
            h_current_level = h0 * M**(-level)
            mesh_parameter_current_level = h_current_level**(-1)
            self.sizes_mesh.append(h_current_level)
            self.mesh_parameters.append(mesh_parameter_current_level)


    '''
    function computing the problem parameters P=[calpha,alpha,cbeta,beta,cgamma,gamma] using least squares fit
    we consider level > 0 to compute calpha,alpha,cbeta,beta for robustness reasons [see PNL17 for details]
    '''
    def ComputeRatesLS(self):
        bias_ratesLS = np.abs(self.difference_QoI.mean)
        variance_ratesLS = self.difference_QoI.sample_variance
        cost_ML_ratesLS = self.time_ML.mean
        mesh_param_ratesLS = self.mesh_parameters[0:self.current_number_levels+1]
        '''mean - alpha
        linear fit'''
        pa = np.polyfit(np.log2(mesh_param_ratesLS[1::]),np.log2(bias_ratesLS[1::]),1)
        alpha   = -pa[0]
        C1      = 2**pa[1]
        '''variance - beta
        linear fit'''
        pb          = np.polyfit(np.log2(mesh_param_ratesLS[1::]),np.log2(variance_ratesLS[1::]),1)
        beta        = -pb[0]
        C2          = 2**pb[1]
        '''cost of computation - gamma
        linear fit'''
        pg          = np.polyfit(np.log2(mesh_param_ratesLS),np.log2(cost_ML_ratesLS),1)
        gamma       = pg[0]
        C3          = 2**pg[1]
        '''update the rates error dictionary'''
        self.rates_error["calpha"] = C1
        self.rates_error["alpha"] = alpha
        self.rates_error["cbeta"] = C2
        self.rates_error["beta"] = beta
        self.rates_error["cgamma"] = C3
        self.rates_error["gamma"] = gamma
        del(bias_ratesLS,variance_ratesLS,cost_ML_ratesLS,mesh_param_ratesLS,C1,C2,C3,alpha,beta,gamma)


    '''
    function performing the Bayesian update of the variance
    using samples generated on all levels in order to locally improve the estimation of Var[difference_QoI]
    '''
    def EstimateBayesianVariance(self,levels): # need to keep levels because in ComputeLevels I use the maximum number of levels
        '''use local variables'''
        k0 = self.settings["k0"]
        k1 = self.settings["k1"]
        calfa = self.rates_error["calpha"]
        alfa  = self.rates_error["alpha"]
        cbeta = self.rates_error["cbeta"]
        beta  = self.rates_error["beta"]
        mesh_param = self.mesh_parameters
        '''use local variables, in order to not modify the global variables'''
        mean_local = self.difference_QoI.mean[:]
        variance_local = self.difference_QoI.sample_variance[:]
        nsam_local = self.number_samples[:]
        '''append null values to evaluate the Bayesian variance for all levels'''
        if len(mean_local) < (levels+1):
            for i in range (0,(levels+1)-len(mean_local)):
                mean_local.append(0.0)
        if len(variance_local) < (levels+1):
            for i in range (0,(levels+1)-len(variance_local)):
                variance_local.append(0.0)
        if len(nsam_local) < (levels+1):
            for i in range ((levels+1)-len(nsam_local)):
                nsam_local.append(0)
        '''estimate the Bayesian variance'''
        BayesianVariance = []
        for level in range (0, (levels+1)):
            mu = calfa*mesh_param[level]**(-alfa)
            lam = (1/cbeta)*mesh_param[level]**(beta)
            G1_l = 0.5 + np.multiply(k1,lam) + np.divide(nsam_local[level],2.0)
            G2_l = k1 + (nsam_local[level]-1)*0.5*variance_local[level] + k0*nsam_local[level]*((mean_local[level]-mu)**2)/(2.0*(k0+nsam_local[level]))
            BayesianVariance.append(np.divide(G2_l,G1_l-0.5))
        self.BayesianVariance = BayesianVariance
        del(BayesianVariance)


    '''
    function computing the minimum number of iterations number_iterations_iE of the MLMC algorithm
    iteration < number_iterations_iE : iterations needed to obtain increasingly accurate estimates of the
                                       problem dependent parameters P=[calpha,alpha,cbeta,beta,cgamma,gamma]
    iteration > number_iterations_iE : iterations preventing redundant computations due to fluctuations
                                       in the estimate of P=[calpha,alpha,cbeta,beta,cgamma,gamma]
                                       by solving the problem for a slightly smaller tolerance than the desired one
    '''
    def ComputeNumberIterationsMLMC(self):
        tolF = self.settings["tolF"]
        tol0 = self.settings["tol0"]
        r2 = self.settings["r2"]
        r1 = self.settings["r1"]
        self.number_iterations_iE = np.floor((-np.log(tolF)+np.log(r2)+np.log(tol0))/(np.log(r1)))


    '''
    function computing the tolerance for the i^th iteration
    '''
    def ComputeTolerancei(self):
        tolF = self.settings["tolF"]
        r2 = self.settings["r2"]
        r1 = self.settings["r1"]
        iE = self.number_iterations_iE
        iter_def = self.current_iteration
        if iter_def < iE:
            tol = (r1**(iE-iter_def) * r2**(-1))*tolF
        elif iter_def > iE:
            tol = (r2**(iE-iter_def) * r2**(-1))*tolF
        else:
            tol = tolF
        self.tolerance_i = tol


    '''
    function computing the number of levels for i^th iteration of the algorithm
    '''
    def ComputeLevels(self):
        tol = self.tolerance_i
        Wmin   = 1e10 # high cost to compare with all the level costs (needs to be a high value)
        current_number_levels = self.current_number_levels
        cgamma = self.rates_error["cgamma"]
        gamma  = self.rates_error["gamma"]
        calpha = self.rates_error["calpha"]
        alpha = self.rates_error["alpha"]
        cphi = self.settings["cphi"]
        mesh_parameters_all_levels = self.mesh_parameters
        Lmax = self.settings["Lmax"]
        Lmin = self.current_number_levels
        '''prepare mesh_parameters_all_levels and BayesianVariance to have both length = Lmax + 1'''
        if len(self.BayesianVariance) < (Lmax+1):
            self.EstimateBayesianVariance(Lmax)
        BayesianVariance = self.BayesianVariance
        model_cost = np.multiply(cgamma,np.power(mesh_parameters_all_levels,gamma)) # model_cost has length = Lmax + 1

        '''observe it is not mandatory to increase the number of levels and we may continue using the number of levels of the previous cmlmc iteration, i.e. Lmin'''
        for lev in range(Lmin, Lmax+1):
            # theta_i = 1.0 - (calpha * (mesh_parameters_all_levels[lev])**(-alpha))/tol # use the ComputeTheta function to evaluate theta for level lev
            self.ComputeTheta(lev)
            theta_i = self.theta_i
            if (theta_i > 0.0) and (theta_i < 1.0):
                coeff2 = np.sum(np.sqrt(np.multiply(model_cost[0:lev+1],BayesianVariance[0:lev+1])))
                coeff2 = coeff2**2.0
                coeff1 = (cphi/(theta_i*tol))**2.0 # formula in case QoI is scalar, if QoI is a field use the formula described in [PNL17]
            else:
                raise Exception ("The splitting parameter theta_i assumed a value outside the range (0,1) :",theta_i)
            Wtot = coeff1 * coeff2
            # print("print level and correspondent cost",lev,Wtot)
            '''change number of levels if the cost condition is satisfied'''
            if Wtot < Wmin:
                Wmin = Wtot
                current_number_levels = lev
        '''add maximum one level per time'''
        if current_number_levels > Lmin:
            current_number_levels = Lmin + 1
        '''update length of Bayesian variance with respect to the current levels'''
        BayesianVariance = BayesianVariance[0:current_number_levels+1]
        '''update variables'''
        self.BayesianVariance = BayesianVariance
        self.current_number_levels = current_number_levels
        self.previous_number_levels = Lmin
        del(tol,Wmin,current_number_levels,cgamma,gamma,calpha,alpha,cphi,mesh_parameters_all_levels,Lmax,Lmin)


    '''
    function computing the splitting parameter theta \in (0,1)
    '''
    def ComputeTheta(self,level):
        calpha = self.rates_error["calpha"]
        alpha = self.rates_error["alpha"]
        tol = self.tolerance_i
        mesh_param = self.mesh_parameters[level]
        self.theta_i = 1.0 - (calpha * (mesh_param)**(-alpha))/tol
        if (self.theta_i < 0.0) or (self.theta_i > 1.0):
            raise Exception ("The splitting parameter theta_i assumed a value outside the range (0,1)")
        del(calpha,alpha,tol,mesh_param)


    '''
    function computing the updated number of samples for each level for the i^th iteration of the cmlmc algorithm
    '''
    def ComputeNumberSamples(self):
        current_number_levels = self.current_number_levels
        BayesianVariance = self.BayesianVariance
        min_samples_add = np.multiply(np.ones(current_number_levels+1),self.settings["minimum_add_level"])
        cgamma = self.rates_error["cgamma"]
        gamma  = self.rates_error["gamma"]
        cphi = self.settings["cphi"]
        mesh_parameters_current_levels = self.mesh_parameters[0:current_number_levels+1]
        theta = self.theta_i
        tol = self.tolerance_i
        nsam = self.number_samples
        '''compute optimal number of samples and store previous number of samples'''
        coeff1 = (cphi/(theta*tol))**2.0
        model_cost = np.multiply(cgamma,np.power(mesh_parameters_current_levels,gamma))
        coeff2 = np.sqrt(np.divide(BayesianVariance,model_cost))
        coeff3 = np.sum(np.sqrt(np.multiply(model_cost,BayesianVariance)))
        opt_number_samples = np.multiply(coeff1*coeff3,coeff2)
        # print("optimal number of samples computed = ",opt_number_samples)
        if (len(opt_number_samples) != current_number_levels+1):
            raise Exception ("length optimal number of samples and current optimal number of level not coherent")
        for lev in range (current_number_levels+1):
            opt_number_samples[lev] = np.ceil(opt_number_samples[lev])
            opt_number_samples[lev] = opt_number_samples[lev].astype(int)
        if len(nsam) < len(opt_number_samples):
            for i in range (len(opt_number_samples)-len(nsam)):
                nsam.append(0)
        '''copy local string and avoid reference since we modify it'''
        previous_number_samples = nsam[:]
        '''evaluate difference between new and previous number of samples'''
        diff_nsam = []
        for lev in range(current_number_levels+1):
            diff_nsam.append(opt_number_samples[lev] - nsam[lev])
            '''set here that if the optimal number of samples is smaller than
            the previous number of samples, keep the previous number of samples,
            i.e. do not decrease the number of samples w.r.t. previous iteration'''
            if diff_nsam[lev] <= 0.:
                diff_nsam[lev] = 0.
                opt_number_samples[lev] = nsam[lev]
            '''set here to have an addition of minimum min_samples_add per level'''
            if (diff_nsam[lev] > 0.) and (diff_nsam[lev] < min_samples_add[lev]):
                diff_nsam[lev] = min_samples_add[lev]
                opt_number_samples[lev] = nsam[lev] + diff_nsam[lev]
            nsam[lev] = opt_number_samples[lev]
        '''convert current number of samples and difference number of samples to integers'''
        for lev in range (current_number_levels+1):
            diff_nsam[lev] = int(diff_nsam[lev])
            nsam[lev] = int(nsam[lev])
        '''update variables and delete local variables'''
        self.number_samples = nsam
        self.difference_number_samples = diff_nsam
        self.previous_number_samples = previous_number_samples
        del(current_number_levels,BayesianVariance,min_samples_add,cgamma,gamma,cphi,mesh_parameters_current_levels,\
        theta,tol,nsam,opt_number_samples,previous_number_samples,diff_nsam)


    '''
    function computing the mlmc estimator for the mean of the Quantity of Interest
    '''
    def compute_mean_mlmc_QoI(self):
        self.mean_mlmc_QoI = np.sum(self.difference_QoI.mean)


    '''
    function computing the total error:
    TErr = bias contrinution + statistical error contribution
    bias contribution B ~= abs(E^MC[Q_{L}-Q_{L-1}])
    statistical error contribution SE = \sum_{i=0}^{L}(Var^MC[Y_l]/N_l)
    '''
    def ComputeTotalErrorMLMC(self):
        self.difference_QoI.bias_error = np.abs(self.difference_QoI.mean[self.current_number_levels])
        variance_from_bayesian = np.zeros(np.size(self.number_samples))
        for lev in range(self.current_number_levels+1):
            variance_from_bayesian[lev] = self.BayesianVariance[lev]/self.number_samples[lev]
        self.difference_QoI.statistical_error = self.settings["cphi"] * np.sqrt(np.sum(variance_from_bayesian))
        TErr = self.difference_QoI.bias_error + self.difference_QoI.statistical_error
        self.TErr = TErr
