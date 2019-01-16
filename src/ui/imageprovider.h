#pragma once

#include "gdx/denseraster.h"
#include "infra/filesystem.h"
#include "infra/geometadata.h"
#include "infra/legend.h"

#include <mutex>
#include <qgeocoordinate.h>
#include <qquickimageprovider.h>
#include <unordered_map>
#include <vector>

namespace opaq {

struct RasterDisplayData
{
    std::shared_ptr<gdx::DenseRaster<double>> raster;
    std::string colorMap;
    inf::Legend legend;
    double zoomLevel;
    QGeoCoordinate coordinate;
};

class RasterDataId
{
public:
    RasterDataId() = default;
    explicit RasterDataId(std::uint64_t idValue) noexcept
    : _value(idValue)
    {
    }

    std::uint64_t value() const noexcept
    {
        return _value;
    }

private:
    std::uint64_t _value = 0;
};

/* This class holds raster data that is about to be fetched by an image provider */
class ImageProviderRasterDataStorage
{
public:
    // place raster data in the data storage
    // returns the id than can be used to retrieved the data
    RasterDataId putRasterData(RasterDisplayData data);

    // retrieve raster data from the data storage
    // the data can only be retrieved once
    RasterDisplayData getRasterData(RasterDataId id);

    void clear();

private:
    std::mutex _mutex;
    uint64_t _nextId = 0;
    std::unordered_map<uint64_t, RasterDisplayData> _data;
};

class AsyncImageProvider : public QQuickAsyncImageProvider
{
public:
    AsyncImageProvider(ImageProviderRasterDataStorage& dataStorage);
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize);

private:
    ImageProviderRasterDataStorage& _dataStorage;
};

}
