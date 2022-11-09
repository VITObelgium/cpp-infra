#include "infra/gdalgeometry.h"

#include <doctest/doctest.h>
#include <fstream>
#include <type_traits>

#include "infra/log.h"

namespace inf::gdal {

using namespace std::string_literals;

TEST_CASE("GdalGeometryTest.fieldDefinition")
{
    static_assert(!std::is_assignable_v<FieldDefinition, FieldDefinitionRef>);
    static_assert(std::is_assignable_v<FieldDefinitionRef, FieldDefinition>);

    FieldDefinition def("Field", typeid(int));

    FieldDefinitionRef defRef(def);
    CHECK("Field"s == defRef.name());
    CHECK(typeid(int) == defRef.type());
}

TEST_CASE("GdalGeometryTest.Ownership")
{
    auto polygon = Geometry::create<Geometry::Type::Polygon>();
    CHECK(Geometry::Type::Polygon == polygon.type());

    {
        PolygonRef polygonRef = polygon;
        CHECK(polygonRef.get() == polygon.get());
    }

    // Reference destroyed, owning object should still be valid
    CHECK(Geometry::Type::Polygon == polygon.type());
}

TEST_CASE("GdalGeometryTest.Slicing")
{
    auto polygon = Geometry::create<Geometry::Type::Polygon>();

    SUBCASE("Ref")
    {
        // The Owner<Polygon> object gets sliced to a GeometryRef object
        GeometryRef geom = polygon;
        CHECK(Geometry::Type::Polygon == geom.type());

        // We can cast the geometry back to PolygonRef
        auto polygonRef = geom.as<PolygonRef>();
        CHECK(polygonRef.get() == polygon.get());
    }

    SUBCASE("CRef")
    {
        // The Owner<Polygon> object gets sliced to a GeometryCRef object
        GeometryRef geom = polygon;
        CHECK(Geometry::Type::Polygon == geom.type());

        // We can cast the geometry back to PolygonCRef
        auto polygonRef = geom.as<PolygonCRef>();
        CHECK(polygonRef.get() == polygon.get());
    }
}

TEST_CASE("Geometry operations")
{
    auto polygon = Geometry::create<Geometry::Type::Polygon>();
    auto ring    = Geometry::create<Geometry::Type::LinearRing>();
    ring.add_point(0.0, 1.0);
    ring.add_point(1.0, 1.0);
    ring.add_point(1.0, 0.0);
    ring.add_point(0.0, 0.0);
    ring.add_point(0.0, 1.0);

    polygon.add_ring(std::move(ring));

    CHECK(polygon.contains(Point(0.5, 0.5)));
    CHECK_FALSE(polygon.contains(Point(1.5, 0.5)));

    auto interiorRing = Geometry::create<Geometry::Type::LinearRing>();
    interiorRing.add_point(0.2, 0.8);
    interiorRing.add_point(0.8, 0.8);
    interiorRing.add_point(0.8, 0.2);
    interiorRing.add_point(0.2, 0.2);
    interiorRing.add_point(0.2, 0.8);
    polygon.add_ring(LinearRingRef(ring));

    CHECK_FALSE(polygon.contains(Point(0.5, 0.5)));
}
}
