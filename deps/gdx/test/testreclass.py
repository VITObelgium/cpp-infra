import os
import gdx
import unittest
import numpy as np


class GdxIntegrationTest(unittest.TestCase):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, 'mapdata', 'reclass')
    reference_dir = os.path.join(script_dir, 'referencedata', 'reclass')

    def map(self, map):
        return gdx.read(os.path.join(self.data_dir, map))

    def ref(self, map):
        return gdx.read(os.path.join(self.reference_dir, map))

    def ref_as(self, type, map):
        return gdx.read_as(type, os.path.join(self.reference_dir, map))

    def test_reclass(self):
        expected = self.ref('ref_reclass.asc')
        result = gdx.reclass(os.path.join(self.data_dir, 'reclass.tab'), self.map('in_key1.asc'))
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclass_nodata(self):
        expected = self.ref('ref_reclass_nodata.asc')
        result = gdx.reclass(os.path.join(self.data_dir, 'reclass_nodata.tab'), self.map('in_key1_nodata.asc'))
        self.assertTrue(gdx.allclose(expected, result))

    def test_reclass_multiple(self):
        expected = self.ref('ref_reclass2.asc')
        result = gdx.reclass(os.path.join(self.data_dir, 'reclass2.tab'), self.map('in_key1.asc'), self.map('in_key2.asc'))
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclass_multiple_nodata(self):
        expected = self.ref('ref_reclass2_nodata.asc')
        result = gdx.reclass(os.path.join(self.data_dir, 'reclass2_nodata.tab'), self.map('in_key1_nodata.asc'), self.map('in_key2_nodata.asc'))
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclassi_index1(self):
        expected = self.ref('ref_reclassi1_1.asc')
        result = gdx.reclassi(os.path.join(self.data_dir, 'reclassi1.tab'), self.map('in_key1.asc'), 1)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclassi_index2(self):
        expected = self.ref('ref_reclassi1_2.asc')
        result = gdx.reclassi(os.path.join(self.data_dir, 'reclassi1.tab'), self.map('in_key1.asc'), 2)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclassi_index1_multiple(self):
        expected = self.ref('ref_reclassi2_1.asc')
        result = gdx.reclassi(os.path.join(self.data_dir, 'reclassi2.tab'), self.map('in_key1.asc'), self.map('in_key2.asc'), 1)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclassi_index2_multiple(self):
        expected = self.ref('ref_reclassi2_2.asc')
        result = gdx.reclassi(os.path.join(self.data_dir, 'reclassi2.tab'), self.map('in_key1.asc'), self.map('in_key2.asc'), 2)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclassi_index1_nodata(self):
        expected = self.ref('ref_reclassi1_1_nodata.asc')
        result = gdx.reclassi(os.path.join(self.data_dir, 'reclassi1_nodata.tab'), self.map('in_key1_nodata.asc'), 1)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_reclassi_index2_nodata(self):
        expected = self.ref('ref_reclassi1_2_nodata.asc')
        result = gdx.reclassi(os.path.join(self.data_dir, 'reclassi1_nodata.tab'), self.map('in_key1_nodata.asc'), 2)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_nreclass(self):
        expected = self.ref_as('float32', 'ref_nreclass.asc')
        result = gdx.nreclass(os.path.join(self.data_dir, 'nreclass.tab'), self.map('in_nreclass.asc'))
        np.testing.assert_array_equal(result.array, expected.array)

    def test_nreclass_nodata(self):
        expected = self.ref_as('float32', 'ref_nreclass_nodata.asc')
        result = gdx.nreclass(os.path.join(self.data_dir, 'nreclass_nodata.tab'), self.map('in_nreclass_nodata.asc'))
        np.testing.assert_allclose(result.array, expected.array)


if __name__ == '__main__':
    unittest.main()
