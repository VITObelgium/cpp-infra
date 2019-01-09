#include "mapview.h"

#include "gdx/denserasterio.h"
#include "imageprovider.h"
#include "infra/log.h"
#include "jobrunner.h"

#include <cassert>
#include <cmath>
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qgeocoordinate.h>
#include <qqmlcontext.h>
#include <qqmlengine.h>
#include <qquickitem.h>
#include <qquickstyle.h>
#include <qwidgetaction.h>

namespace opaq {
using namespace inf;

MapView::MapView(QWidget* parent)
: QWidget(parent)
{
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    _ui.setupUi(this);

    _ui.mapWidget->engine()->addImportPath(QCoreApplication::applicationDirPath() + QDir::separator() + "qml");

    connect(_ui.mapWidget, &QQuickWidget::sceneGraphError, this, [](QQuickWindow::SceneGraphError /*error*/, const QString& message) {
        Log::warn("Scene graph error: {}", message.toStdString());
    });

    connect(_ui.mapWidget, &QQuickWidget::statusChanged, this, [this](QQuickWidget::Status status) {
        if (status == QQuickWidget::Status::Ready) {
            Log::debug("Qml status ready: {}", QMetaEnum::fromType<QQuickWidget::Status>().valueToKey(status));

            auto* object = _ui.mapWidget->rootObject();
            _qmlMap      = object->findChild<QObject*>(QStringLiteral("map"));
            _qmlRaster   = object->findChild<QObject*>(QStringLiteral("raster"));
            assert(_qmlMap);
            assert(_qmlRaster);

            //QObject::connect(_qmlMap, SIGNAL(mouseMoved(QVariant)), this, SLOT(onMouseMoveEvent(QVariant)));
        } else if (status == QQuickWidget::Status::Error) {
            Log::error("Qml error");
            for (auto& error : _ui.mapWidget->errors()) {
                Log::error(error.toString().toStdString());
            }
        }
    });

    setupQml();

    connect(this, &MapView::rasterReadyForDisplay, this, &MapView::onRasterDisplay);
    connect(this, &MapView::rasterOperationFailed, this, &MapView::onRasterOperationFailed);
}

static QGeoRectangle rasterViewPort(const inf::GeoMetadata& meta)
{
    gdal::CoordinateTransformer transformer(meta.projection_epsg().value(), 4326);
    auto topLeft     = meta.top_left();
    auto bottomRight = meta.bottom_right();
    transformer.transform_in_place(topLeft);
    transformer.transform_in_place(bottomRight);
    return QGeoRectangle(QGeoCoordinate(topLeft.y, topLeft.x), QGeoCoordinate(bottomRight.y, bottomRight.x));
}

void MapView::setData(const RasterPtr& data)
{
    setCursor(Qt::WaitCursor);

    _originalDataSource = data;
    JobRunner::queue([this]() {
        processRasterForDisplay(_originalDataSource);
    });

    if (_qmlMap && data->metadata().projection_epsg().has_value()) {
        _qmlMap->setProperty("visibleRegion", QVariant::fromValue(rasterViewPort(data->metadata())));
    }
}

void MapView::setupQml()
{
    _ui.mapWidget->rootContext()->setContextProperty(QStringLiteral("valueprovider"), &_valueProvider);
    _ui.mapWidget->setSource(QUrl(QStringLiteral("qrc:/mapview.qml")));
    _ui.mapWidget->engine()->addImageProvider(QStringLiteral("opaq"), new AsyncImageProvider(_dataStorage));
}

// Calculate the zoom level where one pixel is equal to the cellsize at latitude 0.0
static double calculateZoomLevel(double cellSize)
{
    // source: https://msdn.microsoft.com/en-us/library/bb259689.aspx
    // cellSize = std::cos(latitude * M_PI / 180) * 2 * M_PI * 6378137) / (256 * 2 ^ zoomLevel);

    return std::log2((std::cos(0.0) * 2.0 * M_PI * 6378137.0) / cellSize / 256.0);
}

static RasterDisplayData createRasterDisplayData(const std::string& colorMap, const RasterPtr& raster)
{
    gdal::CoordinateTransformer transformer(raster->metadata().projection_epsg().value(), 4326);
    auto topLeft     = raster->metadata().top_left();
    auto bottomRight = raster->metadata().bottom_right();
    transformer.transform_in_place(topLeft);
    transformer.transform_in_place(bottomRight);

    RasterDisplayData data;
    data.colorMap   = colorMap;
    data.raster     = raster;
    data.zoomLevel  = calculateZoomLevel(raster->metadata().cellSize);
    data.coordinate = QGeoCoordinate(topLeft.y, topLeft.x);
    return data;
}

void MapView::processRasterForDisplay(const RasterPtr& raster)
{
    try {
        _originalDataSource = raster;
        auto warped         = std::make_shared<gdx::DenseRaster<double>>(gdx::warp_raster(*_originalDataSource, 3857));
        auto displayData    = createRasterDisplayData("rdylgn_r", warped);
        auto id             = _dataStorage.putRasterData(displayData);
        _valueProvider.setData(warped);

        emit rasterReadyForDisplay(id, displayData.coordinate, displayData.zoomLevel);
    } catch (const std::exception& e) {
        Log::error("Failed to display raster data: {}", e.what());
        emit rasterOperationFailed();
    }
}

void MapView::onRasterDisplay(RasterDataId id, const QGeoCoordinate& coordinate, double zoomLevel)
{
    auto imageId = QStringLiteral("image://opaq/%1").arg(id.value());
    if (_qmlRaster) {
        _qmlRaster->setProperty("source", imageId);
        _qmlRaster->setProperty("visible", true);
        _qmlRaster->setProperty("coordinate", QVariant::fromValue(coordinate));
        _qmlRaster->setProperty("zoomLevel", zoomLevel);
    } else {
        Log::warn("Raster display failed. Qml error");
    }

    unsetCursor();
}

void MapView::onRasterOperationFailed()
{
    unsetCursor();
}

}
