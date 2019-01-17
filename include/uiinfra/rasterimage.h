#pragma once

#include "infra/filesystem.h"
#include "infra/geometadata.h"

#include <QObject>
#include <mutex>
#include <qquickimageprovider.h>
#include <vector>

#include <QPainter>
#include <QQuickPaintedItem>

namespace inf::ui {

class PixmapImage : public QQuickPaintedItem
{
    Q_OBJECT
public:
    PixmapImage(QQuickItem* parent = nullptr);
    Q_INVOKABLE void setImage(QPixmap pixmapContainer);

protected:
    virtual void paint(QPainter* painter) override;

private:
    QPixmap _pixmap;
};

//template <typename T>
//struct RasterDisplayData
//{
//    std::shared_ptr<T> raster;
//    std::string colorMap;
//};
//
//class RasterDataId
//{
//public:
//    RasterDataId() = default;
//    explicit RasterDataId(std::uint64_t idValue) noexcept
//    : _value(idValue)
//    {
//    }
//
//    std::uint64_t value() const noexcept
//    {
//        return _value;
//    }
//
//private:
//    std::uint64_t _value = 0;
//};
//
///* This class holds raster data that is about to be fetched by an image provider */
//template <typename T>
//class ImageProviderRasterDataStorage
//{
//public:
//    // place raster data in the data storage
//    // returns the id than can be used to retrieved the data
//    RasterDataId putRasterData(RasterDisplayData<T> data)
//    {
//        std::scoped_lock lock(_mutex);
//        auto id = _nextId++;
//        _data.emplace(id, std::move(data));
//        return RasterDataId(id);
//    }
//
//    // retrieve raster data from the data storage
//    // the data can only be retrieved once
//    RasterDisplayData getRasterData(RasterDataId id);
//
//    void clear();
//
//private:
//    std::mutex _mutex;
//    uint64_t _nextId = 0;
//    std::unordered_map<uint64_t, RasterDisplayData<T>> _data;
//};
//
//template <typename T>
//class AsyncImageProvider : public QQuickAsyncImageProvider
//{
//public:
//    AsyncImageProvider(ImageProviderRasterDataStorage<T>& dataStorage);
//    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize);
//
//private:
//    ImageProviderRasterDataStorage<T>& _dataStorage;
//};

}
