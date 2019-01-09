#include "imageprovider.h"

#include "infra/cast.h"
#include "infra/colormap.h"
#include "infra/exception.h"
#include "infra/log.h"
#include "jobrunner.h"

#include "gdx/algo/algorithm.h"
#include "gdx/algo/maximum.h"

#include <future>

namespace opaq {

using namespace inf;

static QImage createRasterImage(const RasterDisplayData& data, QSize* size)
{
    if (!data.raster) {
        return QImage();
    }

    //auto maxValue = iter->upperBound;
    //auto maxColor = iter->color;

    auto cmap     = ColorMap::create(data.colorMap);
    auto maxValue = gdx::maximum(*data.raster);

    auto rgbaData = std::make_unique<std::vector<Color>>(data.raster->size());
    std::transform(optional_value_begin(*data.raster), optional_value_end(*data.raster), rgbaData->begin(), [&](const auto& value) {
        if (value.is_nodata()) {
            return Color{0, 0, 0, 0};
        }

        return cmap.get_color(truncate<float>(*value / maxValue));
    });

    auto width  = data.raster->cols();
    auto height = data.raster->rows();

    if (size) {
        size->setHeight(height);
        size->setWidth(width);
    }

    auto dataPtr = rgbaData.release();
    return QImage(
        reinterpret_cast<uint8_t*>(dataPtr->data()), width, height, QImage::Format_RGBA8888, [](void* data) {
            delete reinterpret_cast<std::vector<Color>*>(data);
        },
        dataPtr);
}

RasterDataId ImageProviderRasterDataStorage::putRasterData(RasterDisplayData data)
{
    std::scoped_lock lock(_mutex);
    auto id = _nextId++;
    _data.emplace(id, std::move(data));
    return RasterDataId(id);
}

RasterDisplayData ImageProviderRasterDataStorage::getRasterData(RasterDataId id)
{
    std::scoped_lock lock(_mutex);
    auto iter = _data.find(id.value());
    if (iter == _data.end()) {
        throw InvalidArgument("Invalid raster data storage id {}", id.value());
    }

    return _data.extract(iter).mapped();
}

void ImageProviderRasterDataStorage::clear()
{
    std::scoped_lock lock(_mutex);
    _nextId = 0;
    _data.clear();
}

class AsyncImageResponse : public QQuickImageResponse
{
public:
    AsyncImageResponse(const QString& id, QSize requestedSize)
    : _id(id)
    , _requestedSize(requestedSize)
    {
    }

    QQuickTextureFactory* textureFactory() const
    {
        return QQuickTextureFactory::textureFactoryForImage(_image);
    }

    void run(const RasterDisplayData& data)
    {
        JobRunner::queue([=]() {
            if (data.raster) {
                try {
                    _image = createRasterImage(data, nullptr);

                    if (_requestedSize.isValid()) {
                        _image = _image.scaled(_requestedSize);
                    }

                } catch (const std::exception& e) {
                    Log::error("Failed to create image from raster: {}", e.what());
                    _error = QString::fromLocal8Bit(e.what());
                }
            }

            emit finished();
        });
    }

    QString errorString() const
    {
        return _error;
    }

private:
    QString _id;
    QString _error;
    QSize _requestedSize;
    QImage _image;
};

AsyncImageProvider::AsyncImageProvider(ImageProviderRasterDataStorage& dataStorage)
: QQuickAsyncImageProvider()
, _dataStorage(dataStorage)
{
}

QQuickImageResponse* AsyncImageProvider::requestImageResponse(const QString& idString, const QSize& requestedSize)
{
    auto* response = new AsyncImageResponse(idString, requestedSize);
    RasterDataId id(idString.toULongLong());

    try {
        auto data = _dataStorage.getRasterData(id);
        response->run(data);
    } catch (const std::exception& e) {
        Log::warn("Image request failed: {}", e.what());
    }

    return response;
}

}
