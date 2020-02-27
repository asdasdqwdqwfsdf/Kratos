from __future__ import print_function, absolute_import, division
import KratosMultiphysics
import KratosMultiphysics.IgaApplication as KratosIGA
import KratosMultiphysics.KratosUnittest as KratosUnittest

class TestTesselation(KratosUnittest.TestCase):
    def test_basic_tesselation(self):

        
        model = KratosMultiphysics.Model()
        iga_model_part =  model.CreateModelPart("IgaModelPart")

        

        with open("geometry.json",'r') as geometry_file:
            iga_geometry_parameters = KratosMultiphysics.Parameters( geometry_file.read())

        iga_geometry_reader = KratosIGA.BrepJsonIO()

        embedded_iga_modeler = KratosIGA.EmbeddedIgaModeler(iga_model_part)
        embedded_iga_modeler.ImportGeometry(iga_geometry_reader, iga_geometry_parameters)
        
        reference_results = [[0.0, 0.0], [0.02454369260617026, 0.0], [0.04908738521234052, 0.0], [0.07363107781851078, 0.0], [0.09817477042468103, 0.0], [0.1227184630308513, 0.0], [0.14726215563702155, 0.0], [0.1718058482431918, 0.0], [0.19634954084936207, 0.0], [0.22089323345553233, 0.0], [0.2454369260617026, 0.0], [0.2699806186678728, 0.0], [0.2945243112740431, 0.0], [0.3190680038802134, 0.0], [0.3436116964863836, 0.0], [0.36815538909255385, 0.0], [0.39269908169872414, 0.0], [0.4172427743048944,0.0], [0.44178646691106466, 0.0], [0.4663301595172349, 0.0], [0.4908738521234052, 0.0], [0.5154175447295755, 0.0], [0.5399612373357456, 0.0], [0.5645049299419159, 0.0], [0.5890486225480862,0.0], [0.6135923151542565, 0.0], [0.6381360077604268, 0.0], [0.662679700366597, 0.0], [0.6872233929727672, 0.0], [0.7117670855789376, 0.0], [0.7363107781851077, 0.0], [0.760854470791278, 0.0], [0.7853981633974483, 0.0], [0.8099418560036186, 0.0], [0.8344855486097889, 0.0], [0.859029241215959, 0.0], [0.8835729338221293, 0.0], [0.9081166264282996, 0.0], [0.9326603190344698, 0.0], [0.9572040116406401, 0.0], [0.9817477042468103, 0.0], [1.0062913968529805, 0.0], [1.030835089459151, 0.0], [1.055378782065321, 0.0], [1.0799224746714913, 0.0], [1.1044661672776617, 0.0], [1.1290098598838318, 0.0], [1.1535535524900022, 0.0], [1.1780972450961724, 0.0], [1.2026409377023426, 0.0], [1.227184630308513, 0.0], [1.2517283229146832, 0.0], [1.2762720155208536, 0.0], [1.3008157081270237, 0.0], [1.325359400733194, 0.0], [1.3499030933393643, 0.0], [1.3744467859455345, 0.0], [1.3989904785517047, 0.0], [1.4235341711578753, 0.0], [1.4480778637640452, 0.0], [1.4726215563702154, 0.0], [1.4971652489763858, 0.0], [1.521708941582556, 0.0], [1.5462526341887264,0.0], [1.5707963267948966, 0.0], [1.5707963267948966, 1.0], [1.5462526341887264, 1.0], [1.521708941582556, 1.0], [1.4971652489763858, 1.0], [1.4726215563702154, 1.0], [1.4480778637640452, 1.0], [1.4235341711578753, 1.0], [1.3989904785517047, 1.0], [1.3744467859455345, 1.0], [1.3499030933393643, 1.0], [1.325359400733194, 1.0], [1.3008157081270237, 1.0], [1.2762720155208536, 1.0], [1.2517283229146832, 1.0], [1.227184630308513, 1.0], [1.2026409377023426, 1.0], [1.1780972450961724, 1.0], [1.1535535524900022, 1.0], [1.1290098598838318, 1.0], [1.1044661672776617, 1.0],[1.0799224746714913, 1.0], [1.055378782065321, 1.0], [1.030835089459151, 1.0], [1.0062913968529807, 1.0], [0.9817477042468103, 1.0], [0.9572040116406401, 1.0], [0.9326603190344698, 1.0], [0.9081166264282996, 1.0], [0.8835729338221293, 1.0], [0.859029241215959, 1.0], [0.8344855486097889, 1.0], [0.8099418560036186, 1.0], [0.7853981633974483, 1.0], [0.760854470791278, 1.0], [0.7363107781851077, 1.0], [0.7117670855789376, 1.0], [0.6872233929727672, 1.0], [0.662679700366597,1.0], [0.6381360077604268, 1.0], [0.6135923151542565, 1.0], [0.5890486225480862, 1.0], [0.564504929941916, 1.0], [0.5399612373357456, 1.0], [0.5154175447295755, 1.0], [0.4908738521234053, 1.0], [0.4663301595172349, 1.0], [0.44178646691106477, 1.0], [0.4172427743048943, 1.0], [0.39269908169872414, 1.0], [0.36815538909255396, 1.0], [0.34361169648638357, 1.0], [0.3190680038802134, 1.0], [0.294524311274043, 1.0], [0.2699806186678728, 1.0], [0.24543692606170264, 1.0], [0.22089323345553225, 1.0], [0.19634954084936207, 1.0], [0.1718058482431919, 1.0], [0.1472621556370215, 1.0], [0.12271846303085132, 1.0], [0.09817477042468115, 1.0], [0.07363107781851075, 1.0], [0.04908738521234057, 1.0], [0.024543692606170175, 1.0], [0.0, 1.0]]

        current_result = embedded_iga_modeler.PrintParametricTessellation()

        self.assertEqual(len(current_result), len(reference_results))

        for reference_result, current_result in zip(reference_results, current_result):
            self.assertAlmostEqual(reference_result, current_result)


if __name__ == '__main__':
    KratosUnittest.main()