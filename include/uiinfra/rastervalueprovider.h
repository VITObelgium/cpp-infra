#pragma once

#include "infra/geometadata.h"
#include "infra/point.h"

#include <optional>
#include <qgeocoordinate.h>
#include <qobject.h>

namespace inf::gdal {
class CoordinateTransformer;
}

namespace inf::ui {

class RasterValueProviderQObject : public QObject
{
    Q_OBJECT

public:
    explicit RasterValueProviderQObject(QObject* parent = nullptr);
    ~RasterValueProviderQObject();

    Q_INVOKABLE double rasterValue(const QGeoCoordinate& coord) const noexcept;
    Q_INVOKABLE QString rasterValueString(const QGeoCoordinate& coord) const noexcept;

protected:
    void setMetadata(const inf::GeoMetadata& meta);

private:
    virtual std::optional<double> getValue(inf::Cell cell) const noexcept = 0;

    inf::GeoMetadata _metadata;
    std::unique_ptr<inf::gdal::CoordinateTransformer> _transformer;
};

template <typename RasterType>
class RasterValueProvider : public RasterValueProviderQObject
{
public:
    explicit RasterValueProvider(QObject* parent = nullptr)
    : RasterValueProviderQObject(parent)
    {
    }

    void setData(const std::shared_ptr<RasterType>& data)
    {
        _raster = data;
        setMetadata(_raster->metadata());
    }

    std::optional<double> getValue(inf::Cell cell) const noexcept
    {
        if (_raster->is_nodata(cell)) {
            return std::optional<double>();
        } else {
            return (*_raster)[cell];
        }
    }

private:
    std::shared_ptr<RasterType> _raster;
};
}
