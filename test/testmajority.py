import os
import gdx
import unittest
import numpy as np


class GdxIntegrationTest(unittest.TestCase):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, 'mapdata', 'majority')
    reference_dir = os.path.join(script_dir, 'referencedata', 'majority')
    cell_size = 100.0

    def map(self, map):
        return gdx.read(os.path.join(self.data_dir, map))

    def byte_map(self, map):
        return gdx.read_as('B', os.path.join(self.data_dir, map))

    def ref(self, map):
        return gdx.read(os.path.join(self.reference_dir, map))

    def byte_ref(self, map):
        return gdx.read_as('B', os.path.join(self.reference_dir, map))

    def test_majority1(self):
        expected = self.ref('out1.asc')
        result = gdx.majority_filter(self.map('in1.asc'), self.cell_size * 2)
        np.testing.assert_allclose(result.array, expected.array, rtol=0.001)

    def test_majority2(self):
        expected = self.ref('out2.asc')
        result = gdx.majority_filter(self.map('in2.asc'), self.cell_size * 1.42)
        np.testing.assert_allclose(result.array, expected.array, rtol=0.001)

    def test_majority3(self):
        expected = self.ref('out3.asc')
        result = gdx.majority_filter(self.map('in3.asc'), self.cell_size * 2)
        self.assertTrue(np.ma.allclose(result.array, expected.array))


if __name__ == '__main__':
    unittest.main()
