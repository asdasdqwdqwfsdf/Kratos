from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# Import applications
import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication

# Import applications and dependencies
import KratosMultiphysics.ParticleMechanicsApplication as KratosParticle

# Importing the base class
from KratosMultiphysics.ParticleMechanicsApplication.mpm_solver import MPMSolver


def CreateSolver(model, custom_settings):
    return MpmExplicitSolver(model, custom_settings)

class MpmExplicitSolver(MPMSolver):
    """The structural mechanics explicit dynamic solver.

    This class creates the mechanical solvers for explicit dynamic analysis.

    See structural_mechanics_solver.py for more information.
    """
    def __init__(self, model, custom_settings):
        # Construct the base solver.
        super(MpmExplicitSolver, self).__init__(model, custom_settings)
        # Lumped mass-matrix is necessary for explicit analysis
        self.material_model_part.ProcessInfo[KratosMultiphysics.COMPUTE_LUMPED_MASS_MATRIX] = True
        self.delta_time_refresh_counter = self.settings["delta_time_refresh"].GetInt()
        KratosMultiphysics.Logger.PrintInfo("::[MpmExplicitSolver]:: Construction finished")

    @classmethod
    def GetDefaultSettings(cls):
        this_defaults = KratosMultiphysics.Parameters("""{
            "scheme_type"                : "central_differences",
            "time_step_prediction_level" : 0,
            "delta_time_refresh"         : 1000,
            "max_delta_time"             : 1.0e0,
            "fraction_delta_time"        : 0.333333333333333333333333333333333333,
            "rayleigh_alpha"             : 0.0,
            "rayleigh_beta"              : 0.0
        }""")
        this_defaults.AddMissingParameters(super(MpmExplicitSolver, cls).GetDefaultSettings())
        return this_defaults

    def AddVariables(self):
        super(MpmExplicitSolver, self).AddVariables()
        self._add_dynamic_variables(self.grid_model_part)

        scheme_type = self.settings["scheme_type"].GetString()
        if(scheme_type == "central_differences"):
            self.grid_model_part.AddNodalSolutionStepVariable(StructuralMechanicsApplication.MIDDLE_VELOCITY)
        if(scheme_type == "multi_stage"):
            self.grid_model_part.AddNodalSolutionStepVariable(StructuralMechanicsApplication.FRACTIONAL_ACCELERATION)
            if (self.settings["rotation_dofs"].GetBool()):
                self.material_model_part.AddNodalSolutionStepVariable(StructuralMechanicsApplication.FRACTIONAL_ANGULAR_ACCELERATION)

        self.grid_model_part.AddNodalSolutionStepVariable(KratosMultiphysics.NODAL_MASS)
        self.grid_model_part.AddNodalSolutionStepVariable(KratosMultiphysics.FORCE_RESIDUAL)
        self.grid_model_part.AddNodalSolutionStepVariable(KratosMultiphysics.RESIDUAL_VECTOR)

        #if (self.settings["rotation_dofs"].GetBool()):
        #    self.material_model_part.AddNodalSolutionStepVariable(StructuralMechanicsApplication.NODAL_INERTIA)
        #    self.material_model_part.AddNodalSolutionStepVariable(KratosMultiphysics.MOMENT_RESIDUAL)

        KratosMultiphysics.Logger.PrintInfo("::[MpmExplicitSolver]:: Variables ADDED")

    def AddDofs(self):
        super(MpmExplicitSolver, self).AddDofs()
        self._add_dynamic_dofs()
        KratosMultiphysics.Logger.PrintInfo("::[MpmExplicitSolver]:: DOF's ADDED")

    def ComputeDeltaTime(self):
        if self.settings["time_step_prediction_level"].GetInt() > 1:
            if self.delta_time_refresh_counter >= self.settings["delta_time_refresh"].GetInt():
                self.delta_time = StructuralMechanicsApplication.CalculateDeltaTime(self.GetComputingModelPart(), self.delta_time_settings)
                self.delta_time_refresh_counter = 0
            else:
                self.delta_time_refresh_counter += 1
        return self.delta_time

    def Initialize(self):
        # Using the base Initialize
        super(MpmExplicitSolver, self).Initialize()

        # Initilize delta_time
        self.delta_time_settings = KratosMultiphysics.Parameters("""{}""")
        self.delta_time_settings.AddValue("time_step_prediction_level", self.settings["time_step_prediction_level"])
        self.delta_time_settings.AddValue("max_delta_time", self.settings["max_delta_time"])
        if self.settings["time_step_prediction_level"].GetInt() > 0:
            self.delta_time = StructuralMechanicsApplication.CalculateDeltaTime(self.GetComputingModelPart(), self.delta_time_settings)
        else:
            self.delta_time = self.settings["time_stepping"]["time_step"].GetDouble()

    #### Specific internal functions ####
    def _create_solution_scheme(self):
        scheme_type = self.settings["scheme_type"].GetString()

        # Setting the Rayleigh damping parameters
        process_info = self.material_model_part.ProcessInfo
        process_info[StructuralMechanicsApplication.RAYLEIGH_ALPHA] = self.settings["rayleigh_alpha"].GetDouble()
        process_info[StructuralMechanicsApplication.RAYLEIGH_BETA] = self.settings["rayleigh_beta"].GetDouble()

        # Setting the time integration schemes
        if(scheme_type == "central_differences"):
            mechanical_scheme = StructuralMechanicsApplication.ExplicitCentralDifferencesScheme(self.settings["max_delta_time"].GetDouble(),
                                                                             self.settings["fraction_delta_time"].GetDouble(),
                                                                             self.settings["time_step_prediction_level"].GetDouble())
        elif(scheme_type == "multi_stage"):
            mechanical_scheme = StructuralMechanicsApplication.ExplicitMultiStageKimScheme(self.settings["fraction_delta_time"].GetDouble())

        else:
            err_msg =  "The requested scheme type \"" + scheme_type + "\" is not available!\n"
            err_msg += "Available options are: \"central_differences\", \"multi_stage\""
            raise Exception(err_msg)
        return mechanical_scheme

    def _create_solution_strategy(self):
        computing_model_part = self.GetComputingModelPart()
        mechanical_scheme = self.get_solution_scheme()

        mechanical_solution_strategy = StructuralMechanicsApplication.MechanicalExplicitStrategy(computing_model_part,
                                            mechanical_scheme,
                                            self.settings["compute_reactions"].GetBool(),
                                            False,
                                            self.settings["move_mesh_flag"].GetBool())

        mechanical_solution_strategy.SetRebuildLevel(0)
        return mechanical_solution_strategy

    #### Private functions ####
