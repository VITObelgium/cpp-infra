import os
import gdx
import unittest
import numpy as np


class GdxIntegrationTest(unittest.TestCase):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, 'mapdata', 'distance')
    reference_dir = os.path.join(script_dir, 'referencedata', 'distance')

    def map(self, map):
        return gdx.read(os.path.join(self.data_dir, map))

    def byte_map(self, map):
        return gdx.read_as('B', os.path.join(self.data_dir, map))

    def ref(self, map):
        return gdx.read(os.path.join(self.reference_dir, map))

    def byte_ref(self, map):
        return gdx.read_as('B', os.path.join(self.reference_dir, map))

    def test_distance(self):
        expected = self.ref('ref_cell_distance.asc')
        result = gdx.distance(self.byte_map('targets.asc'))
        np.testing.assert_allclose(result.array, expected.array, rtol=0.001)

    def test_travel_distance(self):
        expected = self.ref('ref_cell_travel_distance.asc')
        result = gdx.travel_distance(self.byte_map('targets.asc'), self.map('traveltimes.asc'))
        np.testing.assert_allclose(result.array, expected.array, rtol=0.001)

    def test_closest_travel_target(self):
        expected = self.byte_ref('ref_closest_target.asc')
        result = gdx.closest_target(self.byte_map('targets.asc'))
        np.testing.assert_array_equal(result.array, expected.array)

    def test_value_at_closest_target(self):
        expected = self.ref('ref_value_at_closest_target.asc')
        result = gdx.value_at_closest_target(self.byte_map('targets.asc'), self.map('values.asc'))
        np.testing.assert_array_equal(result.array, expected.array)

    def test_sum_within_travel_distance_include_adjacent(self):
        expected = self.ref('ref_sum_in_travel_distance1.asc')
        result = gdx.sum_within_travel_distance(self.map('targets.asc'), self.map('traveltimes.asc'), self.map('travelvalues.asc'), 10, True)
        np.testing.assert_array_equal(result.array, expected.array)

    def test_sum_within_travel_distance_dont_include_adjacent(self):
        expected = self.ref('ref_sum_in_travel_distance0.asc')
        result = gdx.sum_within_travel_distance(self.map('targets.asc'), self.map('traveltimes.asc'), self.map('travelvalues.asc'), 10, False)
        np.testing.assert_array_equal(result.array, expected.array)

    # def test_value_at_closest_travel_target(self):
    #     expected = self.byte_ref('ref_value_at_closest_travel_target.asc')
    #     result = gdx.value_at_closest_travel_target(self.byte_map('targets.asc'), self.map('traveltimes.asc'), self.map('values.asc'))
    #     self.assertTrue(gdx.raster_equal(expected, result))


if __name__ == '__main__':
    unittest.main()
