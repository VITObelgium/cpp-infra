import os
import gdxgpu as gdx
import copy
import math
import unittest
import numpy as np

def create_test_file(path, contents):
    if os.path.exists(path):
        os.remove(path)

    dir = os.path.dirname(path)
    if dir and not os.path.exists(dir):
        os.makedirs(dir)

    with open(path, "w") as text_file:
        text_file.write(contents)


def gen_raster(fill_value):
    return (
        'NCOLS 5\n'
        'NROWS 4\n'
        'XLLCORNER 0.000000\n'
        'YLLCORNER -10.000000\n'
        'CELLSIZE 2.000000\n'
        'NODATA_VALUE -1\n'
        '  {0}  {0}  {0}  {0}  {0}\n'
        '  {0}  {0}  {0}  {0}  {0}\n'
        '  {0}  {0}  {0}  {0}  {0}\n'
        '  {0}  {0}  {0}  {0}  {0}\n'.format(fill_value)
    )

test_raster = (
    'NCOLS 5\n'
    'NROWS 4\n'
    'XLLCORNER 0.000000\n'
    'YLLCORNER -10.000000\n'
    'CELLSIZE 2.000000\n'
    'NODATA_VALUE -1\n'
    '  0  1  2  3  4\n'
    '  5  6  7  8  9\n'
    '  0  0  0  0  0\n'
    '  0  0 -1 -1  0\n'
)

test_array = np.array([[0,  1,  2,  3,  4],
                       [5,  6,  7,  8,  9],
                       [0,  0,  0,  0,  0],
                       [0,  0, float('nan'), float('nan'),  0]], dtype='f')

class TestGdx(unittest.TestCase):
    def test_read_map(self):
        create_test_file('raster.asc', test_raster)
        raster = gdx.read('raster.asc')

        self.assertEqual(5, raster.meta_data.cols)
        self.assertEqual(4, raster.meta_data.rows)
        self.assertEqual(2.0, raster.meta_data.cell_size)
        self.assertEqual(0.0, raster.meta_data.xll)
        self.assertEqual(-10.0, raster.meta_data.yll)
        self.assertEqual(-1, raster.meta_data.nodata_value)
        self.assertTrue(np.allclose(test_array, raster.ndarray, equal_nan=True))

    def test_write_map(self):
        create_test_file('raster.asc', test_raster)
        raster = gdx.read('raster.asc')
        gdx.write(raster, 'writtenraster.asc')

        written_raster = gdx.read('writtenraster.asc')
        self.assertTrue(raster.tolerant_equal(written_raster))

    def test_sum_raster(self):
        create_test_file('raster3.asc', gen_raster(3))
        create_test_file('raster4.asc', gen_raster(4))
        create_test_file('raster7.asc', gen_raster(7))

        raster3 = gdx.read('raster3.asc')
        raster4 = gdx.read('raster4.asc')
        raster7 = gdx.read('raster7.asc')

        sum = raster3 + raster4

        self.assertEqual(sum, raster7)

if __name__ == '__main__':
    unittest.main()

