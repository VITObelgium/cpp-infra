#include "uiinfra/rastervalueprovider.h"

#include "infra/crs.h"
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
    try {
        std::optional<Point<double>> mapCoord;
        std::optional<double> rasterValue;
        bool isOnMap = false;

        if (_transformer) {
            mapCoord  = _transformer->transform(Point<double>(coord.longitude(), coord.latitude()));
            auto cell = _metadata.convert_xy_to_cell(mapCoord->x, mapCoord->y);
            isOnMap   = _metadata.is_on_map(cell);
            if (isOnMap) {
                rasterValue = getValue(cell);
            }
        }

        if (_displayTransformer) {
            mapCoord = _displayTransformer->transform(Point<double>(coord.longitude(), coord.latitude()));
        }

        if (mapCoord.has_value()) {
            if (isOnMap) {
                if (rasterValue.has_value()) {
                    return QString("(X: %1, Y: %2) %3 %4").arg(mapCoord->x, 0, 'f', _decimalsCoord).arg(mapCoord->y, 0, 'f', _decimalsCoord).arg(*rasterValue, 0, 'f', _decimals).arg(_unit.c_str());
                } else {
                    return QString("(X: %1, Y: %2) NODATA").arg(mapCoord->x, 0, 'f', _decimalsCoord).arg(mapCoord->y, 0, 'f', _decimalsCoord);
                }
            } else {
                return QString("(X: %1, Y: %2)").arg(mapCoord->x, 0, 'f', _decimalsCoord).arg(mapCoord->y, 0, 'f', _decimalsCoord);
            }
        }
    } catch (const std::exception& e) {
        Log::debug("Coordinate transform error: {}", e.what());
    }

    return QString();
}

void RasterValueProviderQObject::setUnit(std::string_view unit)
{
    _unit = unit;
}

void RasterValueProviderQObject::setPrecision(int decimals)
{
    _decimals = decimals;
}

void RasterValueProviderQObject::setDisplayEpsg(int32_t epsg)
{
    _displayTransformer = std::make_unique<gdal::CoordinateTransformer>(crs::epsg::WGS84, epsg);
}

void RasterValueProviderQObject::setCoordinatePrecision(double cellsize)
{
    if (cellsize > 0.0 && cellsize < 1.0) {
        _decimalsCoord = std::ceil(-std::log10(cellsize));
    } else {
        _decimalsCoord = 0;
    }
}

void RasterValueProviderQObject::setMetadata(const inf::GeoMetadata& meta)
{
    _metadata = meta;

    if (auto epsg = _metadata.projected_epsg(); epsg.has_value()) {
        _transformer = std::make_unique<gdal::CoordinateTransformer>(crs::epsg::WGS84, epsg.value());
    } else {
        _transformer.reset();
    }
}

void RasterValueProviderQObject::clearMetadata()
{
    _transformer.reset();
}
}
