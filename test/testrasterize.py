import os
import unittest
import gdx
import numpy as np


class GdxIntegrationTest(unittest.TestCase):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, 'mapdata', 'rasterize')
    reference_dir = os.path.join(script_dir, 'referencedata', 'rasterize')

    def ras(self, ras):
        return gdx.read(os.path.join(self.data_dir, ras))

    def ref(self, ras):
        return gdx.read(os.path.join(self.reference_dir, ras))

    def test_draw_shape(self):
        expected = self.ref('ref_draw_shape.tif')
        ras = gdx.raster(expected.metadata, dtype='B', fill=0)
        gdx.draw_shape_on_raster(ras, os.path.join(self.data_dir, 'wegenvl3.shp'))
        np.testing.assert_array_equal(ras.array, expected.array)


if __name__ == '__main__':
    unittest.main()
