#include "infra/gdalalgo.h"

#include <gdal_alg.h>

namespace infra::gdal {

DataSet polygonize(const DataSet& ds)
{
    auto memDriver = gdal::Driver::create(gdal::VectorType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet<int32_t>(0, 0, 0u, "dummy"));
    auto layer = memDataSet.createLayer("Polygons");
    FieldDefinition def("Value", typeid(int32_t));
    layer.createField(def);

    checkError(GDALPolygonize(ds.rasterBand(1).get(), ds.rasterBand(1).get(), layer.get(), 0, nullptr, nullptr, nullptr), "Failed to polygonize raster");
    return memDataSet;
}
}
