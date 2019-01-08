import os
import unittest
import gdx
import numpy as np


class GdxIntegrationTest(unittest.TestCase):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, 'mapdata')
    reference_dir = os.path.join(script_dir, 'referencedata')

    def tab(self, tab_file):
        return os.path.join(self.data_dir, tab_file)

    def map(self, map_file):
        return gdx.read(os.path.join(self.data_dir, map_file))

    def ref(self, map_file):
        return gdx.read(os.path.join(self.reference_dir, map_file))

    def test_inhabitants_green_areas(self):
        lu = self.map('landuse.tif')
        green = gdx.reclass(self.tab('lu2green.tab'), lu)
        green_clusters = gdx.cluster_size(green)
        sel_greenclusters = green_clusters >= 30
        green_in_reach = gdx.sum_in_buffer(sel_greenclusters, 1600) > 0
        inw = gdx.read_as('float32', os.path.join(self.data_dir, 'inhabitants.tif'))
        inhabitants_green_areas = inw * green_in_reach

        expected = gdx.read_as('float32', os.path.join(self.reference_dir, 'inhabitants_green_areas_reference.asc'))
        self.assertTrue(gdx.allclose(expected, inhabitants_green_areas, 1e-3))

    def test_food_production(self):
        gewas = self.map('gewas.tif').astype('float32')
        eerste_bod = self.map('eerste_bod.tif')
        bofek_2012 = self.map('bofek_2012.tif')
        lceu = self.map('lceu.tif')

        textuur = gdx.reclass(self.tab('textuur.tab'), bofek_2012)
        profiel = gdx.reclass(self.tab('profiel.tab'), eerste_bod)

        teelt = gdx.raster(gewas.metadata, fill=1, dtype=gewas.dtype)

        bodemgeschiktheid1 = gdx.reclassi(self.tab('bodemgeschiktheid_a.tab'), textuur, teelt, index=1)
        geschikt = bodemgeschiktheid1 != 0
        bodemgeschiktheid2 = bodemgeschiktheid1 + gdx.reclassi(self.tab('bodemgeschiktheid_b.tab'), profiel, teelt, index=1)
        bodemgeschiktheid2 = geschikt * gdx.clip(bodemgeschiktheid2, 1, 5)

        # STAP 4: Corrigeer landbouwgeschiktheid door rekening te houden met overstromingsgevoeligheid
        rendement = gdx.reclass(self.tab('rendement.tab'), bodemgeschiktheid2.astype('float32'))
        correctiefactor2 = gdx.reclassi(self.tab('opbrengstverlies_overstroming.tab'), teelt, gdx.reclass(self.tab('uiterwaarden.tab'), lceu).astype('float32'), index=1)

        # STAP 5: Bepaal de de biofysische geschiktheid en de potentiele voedselproductie
        potentiele_voedselproductie = rendement - correctiefactor2
        potentiele_voedselproductie = gdx.clip_low(potentiele_voedselproductie, 0)
        fysische_geschiktheid = gdx.normalise(potentiele_voedselproductie)

        self.assertTrue(gdx.allclose(self.ref('fysische_geschiktheid_akker.tif'), fysische_geschiktheid, 1e-5))
        self.assertTrue(gdx.allclose(self.ref('potentiele_voedselproductie_akker.tif'), potentiele_voedselproductie, 1e-5))

        # STAP 6: Bepaal de actuele voedselproductie
        teelt_type_binair = gdx.reclass(self.tab('teelt.tab'), gewas) == teelt
        opbrengstpercentage = gdx.reclass(self.tab('opbrengstverlies_beheer.tab'), gdx.reclass(self.tab('landbouwmilieumaatregelen.tab'), gewas))
        actuele_voedselproductie = opbrengstpercentage * teelt_type_binair * potentiele_voedselproductie

        # STAP 7: Bepaal de waarde van de actuele voedselproductie
        self.assertTrue(gdx.allclose(
            self.ref('waarde_actuele_voedselproductie_met_subsidie_akker.tif'),
            gdx.reclassi(self.tab('boekhoudkundige_resultaten.tab'), gdx.nreclass(self.tab('rendementsklasse.tab'), actuele_voedselproductie), teelt, index=1),
            1e-5
        ))

        self.assertTrue(gdx.allclose(
            self.ref('waarde_actuele_voedselproductie_zonder_subsidie_akker.tif'),
            gdx.reclassi(self.tab('boekhoudkundige_resultaten.tab'), gdx.nreclass(self.tab('rendementsklasse.tab'), actuele_voedselproductie), teelt, index=2),
            1e-5
        ))


if __name__ == '__main__':
    unittest.main()
