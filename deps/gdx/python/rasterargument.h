#pragma once

#include "gdx/raster.h"

#include "gdx/algo/maximum.h"
#include "gdx/maskedraster.h"
#include "gdx/rasterio.h"
#include "gdx/rastermetadata.h"

#include <pybind11/pybind11.h>

namespace gdx {

class Raster;

struct NodataType
{
};

// Wrapper class that allows rasters arguments to be instantiated from:
// - An actual raster instance
// - strings: this should be the path of the raster file
// - numeric values: if there is a context raster provided from which we can determine the size and type

class RasterArgument
{
public:
    RasterArgument(pybind11::object ob);

    Raster::RasterVariant& variant();
    Raster::RasterVariant& variant(const Raster& context);
    Raster::RasterVariant& variant(const RasterMetadata& metadata);

    Raster& raster();
    Raster& raster(const Raster& context);
    Raster& raster(const Raster& context, const std::type_info& type);
    Raster& raster(const RasterMetadata& metadata);

    template <typename T>
    MaskedRaster<T>& get()
    {
        return std::get<MaskedRaster<T>>(variant());
    }

    template <typename T>
    MaskedRaster<T>& get(const Raster& context)
    {
        return std::get<MaskedRaster<T>>(variant(context));
    }

private:
    pybind11::object _ob;
    std::unique_ptr<Raster> _raster;
};
}
