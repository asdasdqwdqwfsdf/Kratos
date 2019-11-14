from __future__ import print_function, absolute_import, division
import KratosMultiphysics as KM

import KratosMultiphysics.MappingApplication as KratosMapping
import KratosMultiphysics.KratosUnittest as KratosUnittest

import math

mdpa_file_name_beam    = "mdpa_files/beam_new"
mdpa_file_name_surface = "mdpa_files/surface_new"

def WriteGiDOutput(model_part):
    from gid_output_process import GiDOutputProcess
    gid_output = GiDOutputProcess(model_part,
        "gid_output_"+model_part.Name,
        KM.Parameters("""
            {
                "result_file_configuration" : {
                    "gidpost_flags": {
                        "GiDPostMode"           : "GiD_PostAscii",
                        "WriteDeformedMeshFlag" : "WriteUndeformed",
                        "WriteConditionsFlag"   : "WriteConditions",
                        "MultiFileFlag"         : "SingleFile"
                    },
                    "nodal_results"       : ["DISPLACEMENT"],
                    "gauss_point_results" : []
                }
            }
            """)
        )

    gid_output.ExecuteInitialize()
    gid_output.ExecuteBeforeSolutionLoop()
    gid_output.ExecuteInitializeSolutionStep()
    gid_output.PrintOutput()
    gid_output.ExecuteFinalizeSolutionStep()
    gid_output.ExecuteFinalize()

def WriteVtkOutput(model_part):
    default_parameters = KM.Parameters("""{
        "file_format"                        : "binary",
        "output_precision"                   : 7,
        "output_control_type"                : "step",
        "output_sub_model_parts"             : false,
        "save_output_files_in_folder"        : false,
        "nodal_solution_step_data_variables" : ["DISPLACEMENT"]
    }""")

    vtk_io = KM.VtkOutput(model_part, default_parameters)
    vtk_io.PrintOutput()

class TestBeamMapper(KratosUnittest.TestCase):
    def setUp(self):
        self.current_model = KM.Model()
        self.model_part_beam = self.current_model.CreateModelPart("beam")
        self.model_part_surface = self.current_model.CreateModelPart("surface")

        # list of variables involved in the Mapper-Tests
        self.model_part_beam.AddNodalSolutionStepVariable(KM.DISPLACEMENT)
        self.model_part_beam.AddNodalSolutionStepVariable(KM.ROTATION)

        self.model_part_surface.AddNodalSolutionStepVariable(KM.DISPLACEMENT)

        KM.ModelPartIO(mdpa_file_name_beam).ReadModelPart(self.model_part_beam)
        KM.ModelPartIO(mdpa_file_name_surface).ReadModelPart(self.model_part_surface)
    
    def addDofs(self):
        for node in self.model_part_beam.Nodes:
            node.AddDof(KM.DISPLACEMENT_X)
            node.AddDof(KM.DISPLACEMENT_Y)
            node.AddDof(KM.DISPLACEMENT_Z)
            node.AddDof(KM.ROTATION_X)
            node.AddDof(KM.ROTATION_Y)
            node.AddDof(KM.ROTATION_Z)
        for node in self.model_part_surface.Nodes:
            node.AddDof(KM.DISPLACEMENT_X)
            node.AddDof(KM.DISPLACEMENT_Y)
            node.AddDof(KM.DISPLACEMENT_Z)

    def test_beam_mapper(self):
        mapper_settings = KM.Parameters("""{
            "mapper_type": "beam_mapper",
            "echo_level" : 3,
            "local_coord_tolerance" : 0.25
        }""")

        self.mapper = KratosMapping.MapperFactory.CreateMapper(self.model_part_beam, self.model_part_surface, mapper_settings)

        for node in self.model_part_beam.Nodes:
            lenght_beam = 100
            alfa = 1.0472  # 20° = 0.3491 rad, 40° = 0.6981, 60° = 1.0472 alfa is the slope of the right end
            r = lenght_beam / alfa
            theta_Z = - node.X / r 
            node.SetSolutionStepValue(KM.DISPLACEMENT_X, r * math.sin(-theta_Z) - node.X)
            node.SetSolutionStepValue(KM.DISPLACEMENT_Y, -r + r*math.cos(-theta_Z))
            node.SetSolutionStepValue(KM.DISPLACEMENT_Z, 0)
            node.SetSolutionStepValue(KM.ROTATION_X, 0)
            node.SetSolutionStepValue(KM.ROTATION_Y, 0)
            node.SetSolutionStepValue(KM.ROTATION_Z, theta_Z )
            
        self.mapper.Map(KM.DISPLACEMENT, KM.ROTATION, KM.DISPLACEMENT)
        #self.mapper.Map(KM.ROTATION, KM.DISPLACEMENT)
        #self.mapper.InverseMap(KM.PRESSURE, KM.TEMPERATURE)
        #self.mapper.InverseMap(KM.DISPLACEMENT, KM.TEMPERATURE)
        #self.mapper.InverseMap(KM.DISPLACEMENT_X, KM.TEMPERATURE)
        #self.mapper.InverseMap(KM.DISPLACEMENT_Y, KM.TEMPERATURE)
        #self.mapper.InverseMap(KM.DISPLACEMENT_Z, KM.TEMPERATURE)

        #WriteGiDOutput(self.model_part_beam)
        #WriteGiDOutput(self.model_part_surface)

        WriteVtkOutput(self.model_part_beam)
        WriteVtkOutput(self.model_part_surface)


if __name__ == '__main__':
    KratosUnittest.main()