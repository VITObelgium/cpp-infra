#include "infra/gdalgeometry.h"

#include <fstream>
#include <gtest/gtest.h>
#include <type_traits>

namespace inf::gdal {

TEST(GdalGeometryTest, fieldDefinition)
{
    static_assert(!std::is_assignable_v<FieldDefinition, FieldDefinitionRef>);
    static_assert(std::is_assignable_v<FieldDefinitionRef, FieldDefinition>);

    FieldDefinition def("Field", typeid(int));

    FieldDefinitionRef defRef(def);
    EXPECT_EQ("Field", defRef.name());
    EXPECT_EQ(typeid(int), defRef.type());
}

TEST(GdalGeometryTest, Ownership)
{
    auto polygon = createGeometry<Polygon>();
    EXPECT_EQ(Geometry::Type::Polygon, polygon.type());

    {
        Polygon polygonRef = polygon;
        EXPECT_EQ(polygonRef.get(), polygon.get());
    }

    // Reference destroyed, owning object should still be valid
    EXPECT_EQ(Geometry::Type::Polygon, polygon.type());
}

TEST(GdalGeometryTest, Slicing)
{
    auto polygon = createGeometry<Polygon>();

    // The Owner<Polygon> object gets sliced to a Geometry object
    Geometry geom = polygon;
    EXPECT_EQ(Geometry::Type::Polygon, geom.type());

    // We can cast the geometry back to Polygon
    auto polygonRef = geom.as<Polygon>();
    EXPECT_EQ(polygonRef.get(), polygon.get());
}
}
