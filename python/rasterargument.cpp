#include "rasterargument.h"
#include "gdx/raster.h"
#include "gdxconfig.h"
#include "infra/filesystem.h"

namespace gdx {

namespace py = pybind11;

RasterArgument::RasterArgument(pybind11::object ob)
: _ob(ob)
{
}

Raster& RasterArgument::raster()
{
    if (py::isinstance<Raster>(_ob)) {
        return _ob.cast<Raster&>();
    } else if (py::isinstance<py::str>(_ob)) {
        if (!_raster) {
            auto path = fs::u8path(_ob.cast<std::string>());
            if (!fs::exists(path)) {
                throw InvalidArgument("Provided raster path is not valid: {}", _ob.cast<std::string>());
            }
            _raster = std::make_unique<Raster>(Raster::read(path));
        }

        return *_raster;
    }

    throw InvalidArgument("Could not create raster from argument");
}

Raster& RasterArgument::raster(const Raster& context)
{
    return raster(context, context.type());
}

Raster& RasterArgument::raster(const Raster& context, const std::type_info& type)
{
    if (py::isinstance<Raster>(_ob) || py::isinstance<py::str>(_ob)) {
        return raster();
    } else if (py::isinstance<NodataType>(_ob)) {
        if (!_raster) {
            // Create a raster that contains only nodata
            auto meta   = context.metadata();
            meta.nodata = Raster::nodataForType(type);
            _raster     = std::make_unique<Raster>(meta, type, meta.nodata.value());
        }

        return *_raster;
    } else if (py::isinstance<py::int_>(_ob) || py::isinstance<py::float_>(_ob)) {
        if (!_raster) {
            _raster = std::make_unique<Raster>(context.metadata(), type, _ob.cast<double>());
        }

        return *_raster;
    }

    throw InvalidArgument("Could not create raster from argument");
}

Raster& RasterArgument::raster(const RasterMetadata& metadata)
{
    if (py::isinstance<Raster>(_ob) || py::isinstance<py::str>(_ob)) {
        return raster();
    } else if (py::isinstance<NodataType>(_ob)) {
        if (!_raster) {
            // Create a raster that contains only nodata
            auto meta   = metadata;
            meta.nodata = Raster::nodataForType(typeid(uint8_t));
            _raster     = std::make_unique<Raster>(meta, typeid(uint8_t), meta.nodata.value());
        }

        return *_raster;
    } else if (py::isinstance<py::int_>(_ob)) {
        if (!_raster) {
            // Create a raster that contains only this value
            auto meta = metadata;
            meta.nodata.reset();
            _raster = std::make_unique<Raster>(meta, typeid(int32_t), _ob.cast<int32_t>());
        }

        return *_raster;
    } else if (py::isinstance<py::float_>(_ob)) {
        if (!_raster) {
            // Create a raster that contains only this value
            auto meta = metadata;
            meta.nodata.reset();
            _raster = std::make_unique<Raster>(meta, typeid(double), _ob.cast<double>());
        }

        return *_raster;
    }

    throw InvalidArgument("Could not create raster from argument");
}

Raster::RasterVariant& RasterArgument::variant()
{
    return raster().get();
}

Raster::RasterVariant& RasterArgument::variant(const Raster& context)
{
    return raster(context).get();
}

Raster::RasterVariant& RasterArgument::variant(const RasterMetadata& metadata)
{
    return raster(metadata).get();
}
}
