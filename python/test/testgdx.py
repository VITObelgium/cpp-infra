import os
import gdx
import copy
import unittest
import numpy as np
from pathlib import Path


def create_test_file(path, contents):
    if os.path.exists(path):
        os.remove(path)

    dir = os.path.dirname(path)
    if dir and not os.path.exists(dir):
        os.makedirs(dir)

    with open(path, "w") as text_file:
        text_file.write(contents)


test_raster = (
    "NCOLS 5\n"
    "NROWS 4\n"
    "XLLCORNER 0.000000\n"
    "YLLCORNER -10.000000\n"
    "CELLSIZE 2.000000\n"
    "NODATA_VALUE -1\n"
    "  0  1  2  3  4\n"
    "  5  6  7  8  9\n"
    "  0  0  0  0  0\n"
    "  0  0 -1 -1  0\n"
)

test_array = np.array(
    [
        [0, 1, 2, 3, 4],
        [5, 6, 7, 8, 9],
        [0, 0, 0, 0, 0],
        [0, 0, float("nan"), float("nan"), 0],
    ],
    dtype="f",
)


class TestGdx(unittest.TestCase):
    def test_raster_creation(self):
        meta = gdx.raster_metadata(rows=3, cols=4)
        ras = gdx.raster(meta, dtype=float)
        self.assertEqual(np.dtype(float), ras.dtype)
        self.assertEqual(3, ras.metadata.rows)
        self.assertEqual(4, ras.metadata.cols)

        ras = gdx.raster(rows=3, cols=4, dtype=int)
        self.assertEqual(np.dtype("int32"), ras.dtype)
        self.assertEqual(3, ras.metadata.rows)
        self.assertEqual(4, ras.metadata.cols)

    def test_raster_creation_with_fill_value(self):
        ras = gdx.raster(rows=3, cols=4, dtype=int, fill=4)
        self.assertEqual(np.dtype("int32"), ras.dtype)
        self.assertEqual(3, ras.metadata.rows)
        self.assertEqual(4, ras.metadata.cols)
        self.assertTrue(np.all(ras.array == 4))

        ras = gdx.raster(gdx.raster_metadata(rows=3, cols=4), dtype=int, fill=4)
        self.assertEqual(np.dtype("int32"), ras.dtype)
        self.assertEqual(3, ras.metadata.rows)
        self.assertEqual(4, ras.metadata.cols)
        self.assertTrue(np.all(ras.array == 4))

    def test_raster_creation_with_dtype(self):
        ras = gdx.raster(rows=3, cols=4, dtype=int)
        self.assertEqual(np.dtype("int32"), ras.dtype)

    def test_expand_integer_to_raster(self):
        meta = gdx.raster_metadata(rows=3, cols=4)

        ras = gdx.raster(meta, dtype=float, fill=42.0)
        self.assertTrue(gdx.raster_equal(ras, 42.0))
        self.assertTrue(gdx.raster_equal(ras, 42))
        self.assertFalse(gdx.raster_equal(ras, 6.0))

        ras = gdx.raster(meta, dtype=float, fill=42)
        self.assertTrue(gdx.raster_equal(ras, 42.0))
        self.assertTrue(gdx.raster_equal(ras, 42))

        ras = gdx.raster(meta, dtype=int, fill=42)
        self.assertTrue(gdx.raster_equal(ras, 42))
        self.assertTrue(gdx.raster_equal(ras, 42.0))

    def test_metadata_init(self):
        meta = gdx.raster_metadata()
        self.assertEqual(0, meta.rows)
        self.assertEqual(0, meta.cols)
        self.assertIsNone(meta.nodata)

        meta = gdx.raster_metadata(rows=5, cols=4)
        self.assertEqual(5, meta.rows)
        self.assertEqual(4, meta.cols)
        self.assertIsNone(meta.nodata)

        meta = gdx.raster_metadata(cols=4, rows=5)
        self.assertEqual(5, meta.rows)
        self.assertEqual(4, meta.cols)
        self.assertIsNone(meta.nodata)

        meta = gdx.raster_metadata(cols=4, rows=5, nodata=-9999.0)
        self.assertEqual(5, meta.rows)
        self.assertEqual(4, meta.cols)
        self.assertEqual(-9999.0, meta.nodata)

        meta = gdx.raster_metadata(cols=4, rows=5, cell_size=10, xll=10000, yll=100000)
        self.assertEqual(5, meta.rows)
        self.assertEqual(4, meta.cols)
        self.assertEqual(10000, meta.xll)
        self.assertEqual(100000, meta.yll)
        self.assertEqual(10, meta.cell_size)
        self.assertIsNone(meta.nodata)

    def test_metadata_nodata(self):
        meta = gdx.raster_metadata(rows=5, cols=4)

        self.assertIsNone(meta.nodata)
        meta.nodata = 5.0
        self.assertEqual(5.0, meta.nodata)

    def test_read_map(self):
        create_test_file("raster.asc", test_raster)
        raster = gdx.read_as(float, "raster.asc")

        self.assertEqual(5, raster.metadata.cols)
        self.assertEqual(4, raster.metadata.rows)
        self.assertEqual(2.0, raster.metadata.cell_size)
        self.assertEqual(0.0, raster.metadata.xll)
        self.assertEqual(-10.0, raster.metadata.yll)
        self.assertEqual(-1, raster.metadata.nodata)
        self.assertTrue(np.allclose(test_array, raster.array, equal_nan=True))

    def test_read_map_from_pathlib(self):
        create_test_file("raster.asc", test_raster)
        raster = gdx.read(Path("raster.asc"))
        raster = gdx.read_as(float, Path("raster.asc"))

    def test_write_map(self):
        create_test_file("raster.asc", test_raster)
        raster = gdx.read("raster.asc")
        gdx.write(raster, "writtenraster.asc")

        written_raster = gdx.read("writtenraster.asc")
        os.remove("writtenraster.asc")
        self.assertTrue(np.allclose(raster.array, written_raster.array, equal_nan=True))

    def test_modify_raster_using_numpy(self):
        # modify the raster data using the ndarry accessor
        # verify that the internal raster data has changed by writing it to disk
        # and comparing the result

        create_test_file("raster.asc", test_raster)
        raster = gdx.read("raster.asc")

        raster.array.fill(44)

        gdx.write(raster, "writtenraster.asc")
        written_raster = gdx.read("writtenraster.asc")
        os.remove("writtenraster.asc")

        expected = np.array(
            [
                [44, 44, 44, 44, 44],
                [44, 44, 44, 44, 44],
                [44, 44, 44, 44, 44],
                [44, 44, 44, 44, 44],
            ],
            dtype="f",
        )

        self.assertTrue(np.allclose(expected, written_raster.array, equal_nan=True))

    def test_method_chain(self):
        expected = np.array(
            [[0, 1, 2, 3, 4], [5, 11, 12, 8, 9], [0, 0, 0, 0, 0], [0, 0, 9, 9, 0]],
            dtype=int,
        )

        ras = gdx.raster_from_ndarray(
            test_array, gdx.raster_metadata(rows=4, cols=5, nodata=float("NaN"))
        )
        ras = ras.replace_value(6, 11).replace_value(7, 12)
        np.testing.assert_array_equal(expected, ras.array)

    def test_astype(self):
        ras = gdx.raster_from_ndarray(test_array, gdx.raster_metadata(rows=4, cols=5))
        intras = ras.astype(int)

        expected = test_array.astype("int32")
        np.testing.assert_array_equal(expected, intras.array)

    def test_astype_float_and_back(self):
        byte_array = np.array([[255, 1, 2], [5, 6, 7], [0, 0, 255]], dtype="B")

        meta = gdx.raster_metadata(rows=3, cols=3)
        meta.nodata = 255
        ras = gdx.raster_from_ndarray(byte_array, meta)

        float_raster = ras.astype(float)
        byte_raster = float_raster.astype("B")

        np.testing.assert_array_equal(ras.array, byte_raster.array)
        np.testing.assert_array_equal(byte_array, byte_raster.array)

    def test_sum_raster(self):
        create_test_file("raster.asc", test_raster)
        raster1 = gdx.read("raster.asc")
        raster2 = copy.deepcopy(raster1)

        raster1.array.fill(4)
        raster2.array.fill(3)

        sum = raster1 + raster2

        expected = np.array(
            [[7, 7, 7, 7, 7], [7, 7, 7, 7, 7], [7, 7, 7, 7, 7], [7, 7, 7, 7, 7]],
            dtype="f",
        )

        np.testing.assert_allclose(expected, sum.array, equal_nan=True)

    def test_comparison(self):
        create_test_file("raster.asc", test_raster)
        raster = gdx.read_as(float, "raster.asc")
        result = raster > 5

        expected = np.array(
            [[0, 0, 0, 0, 0], [0, 1, 1, 1, 1], [0, 0, 0, 0, 0], [0, 0, 255, 255, 0]],
            dtype="B",
        )

        self.assertTrue(np.allclose(expected, result.array))

    def test_comparison_int(self):
        create_test_file("raster.asc", test_raster)
        raster = gdx.read_as(int, "raster.asc")
        result = raster != 0

        expected = np.array(
            [[0, 1, 1, 1, 1], [1, 1, 1, 1, 1], [0, 0, 0, 0, 0], [0, 0, 255, 255, 0]],
            dtype="B",
        )

        np.testing.assert_array_equal(expected, result.array)

    def test_create_from_float_array(self):
        array = np.array(
            [
                [1, 0, 0, 0, 0],
                [0, 1, 1, 1, 1],
                [0, 0, 0, 0, 0],
                [0, 0, float("nan"), float("nan"), 0],
            ],
            dtype="f",
        )

        meta = gdx.raster_metadata()
        meta.rows = 4
        meta.cols = 5

        result = gdx.raster_from_ndarray(array, meta)

        self.assertTrue(np.allclose(array, result.array, equal_nan=True))

    def test_create_from_bool_array(self):
        array = np.zeros((2, 2))

        meta = gdx.raster_metadata()
        meta.rows = 2
        meta.cols = 2

        result = gdx.raster_from_ndarray(array == 0, meta)
        np.testing.assert_array_equal(result.array, np.ones((2, 2)))

    def test_create_from_float_array_bad_dimension(self):
        array = np.array(
            [[0, 0, 0, 0, 0], [0, 1, 1, 1, 1], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
            dtype="f",
        )

        meta = gdx.raster_metadata()
        meta.rows = 5
        meta.cols = 4

        with self.assertRaises(RuntimeError):
            gdx.raster_from_ndarray(array, meta)

    def test_create_from_int_array(self):
        array = np.array(
            [[0, 0, 0, 0, 0], [0, 1, 1, 1, 1], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
            dtype="i",
        )

        meta = gdx.raster_metadata(rows=4, cols=5)
        result = gdx.raster_from_ndarray(array, meta)

        self.assertTrue(np.allclose(array, result.array))

    def test_operators_int_raster(self):
        array = np.array(
            [[1, 1, 1, 1, 1], [1, 1, 1, 1, 1], [1, 1, 1, 1, 1], [1, 1, 1, 1, 1]],
            dtype="i",
        )

        # no need to check the values, already tested in c++
        # just check if the binding works
        ras = gdx.raster_from_ndarray(array, gdx.raster_metadata(rows=4, cols=5))
        self.assertEqual(array.dtype, ras.dtype)

        self.assertEqual((ras * 1).dtype, ras.dtype)
        self.assertEqual((ras + 1).dtype, ras.dtype)
        self.assertEqual((ras / 1).dtype, ras.dtype)
        self.assertEqual((ras - 1).dtype, ras.dtype)

        self.assertEqual((1 * ras).dtype, ras.dtype)
        self.assertEqual((1 + ras).dtype, ras.dtype)
        self.assertEqual((1 / ras).dtype, np.dtype("float32"))
        self.assertEqual((1 - ras).dtype, ras.dtype)

        self.assertEqual(gdx.logical_not(ras).dtype, np.dtype("B"))
        self.assertEqual(gdx.logical_and(ras, ras).dtype, np.dtype("B"))
        self.assertEqual(gdx.logical_or(ras, ras).dtype, np.dtype("B"))

        with self.assertRaises(ValueError):
            ras * 1.0

        with self.assertRaises(ValueError):
            ras + 1.0

        with self.assertRaises(ValueError):
            ras / 1.0

        with self.assertRaises(ValueError):
            ras - 1.0

    def test_operators_int_raster_divide_by_zero(self):
        array = np.array(
            [[1, 1, 1, 1, 1], [1, 1, 0, 1, 1], [1, 1, 1, 1, 1], [1, 1, 1, 1, 1]],
            dtype="i",
        )

        ras = gdx.raster_from_ndarray(array, gdx.raster_metadata(rows=4, cols=5))

        with self.assertRaises(ValueError):
            ras / 0

        # with self.assertRaises(RuntimeError):
        #     1 / ras

        # division by zero is allowed when there is a nodata value
        # it will result in nodata
        ras.metadata.nodata = -1
        1 / ras

    def test_operators_equality(self):
        arr1 = gdx.raster_from_ndarray(
            np.ones([2, 3], dtype=int), gdx.raster_metadata(rows=2, cols=3)
        )
        arr2 = gdx.raster_from_ndarray(
            np.ones([2, 3], dtype=int), gdx.raster_metadata(rows=2, cols=3)
        )
        arr3 = gdx.raster_from_ndarray(
            np.ones([2, 3], dtype="B"), gdx.raster_metadata(rows=2, cols=3)
        )

        np.testing.assert_array_equal(np.ones([2, 3], dtype="B"), (arr1 == arr2).array)
        np.testing.assert_array_equal(np.ones([2, 3], dtype="B"), (arr1 == arr3).array)

    def test_operators_float_raster(self):
        array = np.array(
            [[0, 0, 0, 0, 0], [0, 1, 1, 1, 1], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
            dtype="float32",
        )

        # no need to check the values, already tested in c++
        # just check if the binding works
        ras = gdx.raster_from_ndarray(
            array, gdx.raster_metadata(rows=4, cols=5, nodata=-1)
        )
        ras * 1
        ras + 1
        ras / 1
        ras - 1

        1 * ras
        1 + ras
        1 / ras
        1 - ras

        ras * 1.0
        ras + 1.0
        ras / 1.0
        ras - 1.0

        1.0 * ras
        1.0 + ras
        1.0 / ras
        1.0 - ras

    def test_operators_float_raster_divide_by_zero(self):
        array = np.array(
            [[1, 1, 1, 1, 1], [0, 0, 0, 0, 0], [1, 1, 1, 1, 1], [1, 1, 1, 1, 1]],
            dtype="float32",
        )

        ras = gdx.raster_from_ndarray(
            array, gdx.raster_metadata(rows=4, cols=5, nodata=-1)
        )

        with self.assertRaises(ValueError):
            ras / 0.0

        with self.assertRaises(ValueError):
            ras / 0

        1 / ras
        1.0 / ras

    def test_boolean_operator(self):
        meta = gdx.raster_metadata(rows=0, cols=0)
        ras = gdx.raster(meta, float)
        self.assertFalse(ras)

        meta = gdx.raster_metadata(rows=4, cols=5)
        ras = gdx.raster(meta, float)
        self.assertTrue(ras)

    def test_is_nodata(self):
        array = np.ndarray(shape=(2, 2), dtype=float)
        array.fill(1)

        meta = gdx.raster_metadata(rows=2, cols=2)
        meta.nodata = 1

        ras = gdx.raster_from_ndarray(array, meta)
        self.assertTrue(gdx.all(gdx.is_nodata(ras)))

    def test_min_max(self):
        meta = gdx.raster_metadata(rows=3, cols=3)
        array1 = gdx.raster_from_ndarray(np.full((3, 3), 2, dtype=int), meta)
        array2 = gdx.raster_from_ndarray(np.full((3, 3), 4, dtype=int), meta)
        array3 = gdx.raster_from_ndarray(np.full((3, 3), 6, dtype=int), meta)

        max = gdx.max(array1, array2)
        self.assertTrue(np.array_equal(array2.array, max.array))

        max = gdx.max(array1, array2, array3)
        self.assertTrue(np.array_equal(array3.array, max.array))

        min = gdx.min(array1, array2)
        self.assertTrue(np.array_equal(array1.array, min.array))

        min = gdx.min(array2, array3)
        self.assertTrue(np.array_equal(array2.array, min.array))

    def test_equal_one_of(self):
        meta = gdx.raster_metadata(rows=3, cols=3)
        meta.nodata = -1
        ras = gdx.raster_from_ndarray(
            np.array([[1, 2, -1], [4, 5, 6], [-1, 8, 9]], dtype=int), meta
        )

        expected = np.array([[1, 0, -1], [0, 1, 0], [-1, 0, 1]], dtype=int)

        res = gdx.raster_equal_one_of(ras, [1, 5, 9])
        self.assertTrue(np.array_equal(res.array, expected))

    def test_abs(self):
        meta = gdx.raster_metadata(rows=3, cols=3)
        meta.nodata = -1
        ras = gdx.raster_from_ndarray(
            np.array([[-1, 2, -1], [4, -5, 6], [-1, 8, -9]], dtype=int), meta
        )

        expected = np.array([[-1, 2, -1], [4, 5, 6], [-1, 8, 9]], dtype=int)

        res = gdx.abs(ras)
        np.testing.assert_array_equal(res.array, expected)

    def test_round(self):
        meta = gdx.raster_metadata(rows=3, cols=3)
        meta.nodata = -1
        ras = gdx.raster_from_ndarray(
            np.array(
                [[-1, 2.1, -1], [4.4, -5.1, 6.9], [-1, 8.6, -9.8]], dtype="float32"
            ),
            meta,
        )

        expected = np.array(
            [[float("nan"), 2, float("nan")], [4, -5, 7], [float("nan"), 9, -10]],
            dtype="float32",
        )

        res = gdx.round(ras)
        self.assertTrue(np.allclose(res.array, expected, equal_nan=True))

    def test_exp_rasters(self):
        meta = gdx.raster_metadata(rows=2, cols=2)
        meta.nodata = -1
        base = gdx.raster_from_ndarray(np.array([[2, -1], [-1, 3]], dtype=int), meta)
        exp = gdx.raster_from_ndarray(np.array([[4, -1], [-1, 2]], dtype=int), meta)

        res = gdx.pow(base, exp)
        expected = np.array(
            [[16, float("nan")], [float("nan"), 9]], dtype=np.dtype("float32")
        )

        self.assertEqual(expected.dtype, np.dtype("float32"))
        self.assertTrue(np.allclose(res.array, expected, equal_nan=True))

    def test_exp_value_exp(self):
        meta = gdx.raster_metadata(rows=2, cols=2)
        meta.nodata = -1
        base = gdx.raster_from_ndarray(np.array([[2, -1], [-1, 3]], dtype=int), meta)
        res = gdx.pow(base, 2)
        expected = np.array(
            [[4, float("nan")], [float("nan"), 9]], dtype=np.dtype("float32")
        )

        self.assertEqual(expected.dtype, np.dtype("float32"))
        self.assertTrue(np.allclose(res.array, expected, equal_nan=True))

    def test_regression_bug_2018_05_08(self):
        meta = gdx.raster_metadata(rows=2, cols=2)
        meta.nodata = -1
        a = gdx.raster_from_ndarray(np.array([[0, -1], [0.1, 1]], dtype=float), meta)
        b = gdx.if_then_else(
            a < 0.4, 0.4, a
        )  # bug was that the then-part got type uint8
        b = b.array
        expected = np.array([[0.4, float("nan")], [0.4, 1]], dtype=np.dtype("float32"))
        self.assertTrue(np.allclose(b, expected, equal_nan=True))

    def test_replace_nodata(self):
        meta = gdx.raster_metadata(rows=2, cols=2)
        meta.nodata = -1
        a = gdx.raster_from_ndarray(np.array([[0, -1], [0.1, 1]], dtype=float), meta)
        a.replace_nodata(1)
        expected = np.array([[0, 1], [0.1, 1]], dtype=np.dtype("float32"))
        self.assertTrue(np.allclose(a.array, expected, equal_nan=True))

    def test_fuzzy_cluster_id(self):
        meta = gdx.raster_metadata(rows=6, cols=6)
        meta.nodata = -1
        meta.cell_size = 100
        a = gdx.raster_from_ndarray(np.zeros((6, 6), dtype=np.int32), meta)
        a.array[0, 0] = 1
        a.array[2, 2] = 1
        a.array[5, 5] = 1
        b = gdx.fuzzy_cluster_id(a, 2.9 * meta.cell_size)
        expected = np.zeros((6, 6), dtype=np.int32)
        expected[0, 0] = 1
        expected[2, 2] = 1
        expected[5, 5] = 2
        self.assertTrue(np.array_equal(b.array, expected))

    def test_regression_bug_2018_05_09(self):
        meta = gdx.raster_metadata(rows=1, cols=1)
        meta.nodata = -1
        a = gdx.raster_from_ndarray(np.zeros((1, 1), dtype=np.int32), meta)
        b = gdx.if_then_else(a > 0, a, a)
        self.assertTrue(a.metadata.nodata == b.metadata.nodata)


if __name__ == "__main__":
    unittest.main()
