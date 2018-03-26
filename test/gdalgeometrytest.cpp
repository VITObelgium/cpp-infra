#include "infra/gdalgeometry.h"

#include <fstream>
#include <gtest/gtest.h>
#include <type_traits>

namespace infra::gdal {

TEST(GdalGeometryTest, fieldDefinition)
{
    static_assert(!std::is_assignable_v<FieldDefinition, FieldDefinitionRef>);
    static_assert(std::is_assignable_v<FieldDefinitionRef, FieldDefinition>);

    FieldDefinition def("Field", typeid(int));

    FieldDefinitionRef defRef(def);
    EXPECT_EQ("Field", defRef.name());
    EXPECT_EQ(typeid(int), defRef.type());
}
}
