#include "uiinfra/rastervalueprovider.h"

#include "infra/gdal.h"
#include "infra/log.h"

namespace inf::ui {

RasterValueProviderQObject::RasterValueProviderQObject(QObject* parent)
: QObject(parent)
{
}

RasterValueProviderQObject::~RasterValueProviderQObject() = default;

double RasterValueProviderQObject::rasterValue(const QGeoCoordinate& coord) const noexcept
{
    if (_transformer) {
        try {
            Point<double> mapCoord = _transformer->transform(Point<double>(coord.longitude(), coord.latitude()));
            auto cell              = _metadata.convert_xy_to_cell(mapCoord.x, mapCoord.y);
            if (_metadata.is_on_map(cell)) {
                return getValue(cell).value_or(std::numeric_limits<double>::quiet_NaN());
            }
        } catch (const std::exception& e) {
            Log::debug("Coordinate transform error: {}", e.what());
        }
    }

    return std::numeric_limits<double>::quiet_NaN();
}

QString RasterValueProviderQObject::rasterValueString(const QGeoCoordinate& coord) const noexcept
{
    if (_transformer) {
        try {
            auto mapCoord = _transformer->transform(Point<double>(coord.longitude(), coord.latitude()));
            auto cell     = _metadata.convert_xy_to_cell(mapCoord.x, mapCoord.y);
            if (_metadata.is_on_map(cell)) {
                if (auto rasterValue = getValue(cell); rasterValue.has_value()) {
                    return QString("(X: %1, Y: %2) %3").arg(int(mapCoord.x)).arg(int(mapCoord.y)).arg(*rasterValue, 0, 'f', 6);
                } else {
                    return QString("(X: %1, Y: %2) NODATA").arg(int(mapCoord.x)).arg(int(mapCoord.y));
                }
            }
        } catch (const std::exception& e) {
            Log::debug("Coordinate transform error: {}", e.what());
        }
    }

    return QString();
}

void RasterValueProviderQObject::setMetadata(const inf::GeoMetadata& meta)
{
    _metadata = meta;

    if (auto epsg = _metadata.projection_epsg(); epsg.has_value()) {
        _transformer = std::make_unique<gdal::CoordinateTransformer>(4326, epsg.value());
    } else {
        _transformer.reset();
    }
}
}
