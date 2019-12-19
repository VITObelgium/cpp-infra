#include "infra/gdalgeometry.h"

#include <doctest/doctest.h>
#include <fstream>
#include <type_traits>

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
    auto polygon = createGeometry<Polygon>();
    CHECK(Geometry::Type::Polygon == polygon.type());

    {
        Polygon polygonRef = polygon;
        CHECK(polygonRef.get() == polygon.get());
    }

    // Reference destroyed, owning object should still be valid
    CHECK(Geometry::Type::Polygon == polygon.type());
}

TEST_CASE("GdalGeometryTest.Slicing")
{
    auto polygon = createGeometry<Polygon>();

    // The Owner<Polygon> object gets sliced to a Geometry object
    Geometry geom = polygon;
    CHECK(Geometry::Type::Polygon == geom.type());

    // We can cast the geometry back to Polygon
    auto polygonRef = geom.as<Polygon>();
    CHECK(polygonRef.get() == polygon.get());
}
}
