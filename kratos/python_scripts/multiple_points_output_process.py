from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# other imports
from point_output_process import PointOutputProcess

def Factory(settings, Model):
    if(type(settings) != KratosMultiphysics.Parameters):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return MultiplePointsOutputProcess(Model, settings["Parameters"])

class MultiplePointsOutputProcess(KratosMultiphysics.Process):
    """This process writes several points to a file
    Internally it holds objects of type "PointOutputProcess"
    """
    def __init__(self, model, params):

        default_settings = KratosMultiphysics.Parameters('''{
            "positions"         : [[]],
            "model_part_name"  : "",
            "output_file_name" : "",
            "output_variables" : [],
            "entity_type"      : "element"
        }''')

        params.ValidateAndAssignDefaults(default_settings)

        positions = params["positions"].GetMatrix()

        num_points = positions.Size1()
        if num_points == 0:
            raise Exception('No positions were specified"')
        if positions.Size2() != 3:
            raise Exception('The positions have to be provided with 3 coordinates!')

        output_file_name_base = params["output_file_name"].GetString()
        if output_file_name_base == "":
            raise Exception('No "output_file_name" was specified!')

        self.point_output_processes = []
        # Create the individual point_output_processes
        params.RemoveValue("positions")
        params.AddEmptyValue("position")
        position_vec = KratosMultiphysics.Vector(3)
        for i in range(num_points):
            point_proc_params = params.Clone()

            for j in range(3):
                position_vec[j] = positions[i,j]
            point_proc_params["position"].SetVector(position_vec)

            output_file_name = output_file_name_base + "_" + str(i+1) # TODO come up with sth better?
            point_proc_params["output_file_name"].SetString(output_file_name)

            self.point_output_processes.append(PointOutputProcess(model, point_proc_params))

    def ExecuteInitialize(self):
        for proc in self.point_output_processes:
            proc.ExecuteInitialize()

    def ExecuteBeforeSolutionLoop(self):
        for proc in self.point_output_processes:
            proc.ExecuteBeforeSolutionLoop()

    def ExecuteInitializeSolutionStep(self):
        for proc in self.point_output_processes:
            proc.ExecuteInitializeSolutionStep()

    def ExecuteFinalizeSolutionStep(self):
        for proc in self.point_output_processes:
            proc.ExecuteFinalizeSolutionStep()

    def ExecuteBeforeOutputStep(self):
        for proc in self.point_output_processes:
            proc.ExecuteBeforeOutputStep()

    def IsOutputStep(self):
        return True # we always print output

    def PrintOutput(self):
        for proc in self.point_output_processes:
            proc.PrintOutput()

    def ExecuteAfterOutputStep(self):
        for proc in self.point_output_processes:
            proc.ExecuteAfterOutputStep()

    def ExecuteFinalize(self):
        for proc in self.point_output_processes:
            proc.ExecuteFinalize()
