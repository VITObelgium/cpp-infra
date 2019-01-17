#include "mapview.h"

#include "gdx/algo/algorithm.h"
#include "gdx/algo/maximum.h"
#include "gdx/algo/minimum.h"
#include "gdx/denserasterio.h"
#include "infra/log.h"
#include "jobrunner.h"
#include "rasterdisplaydata.h"
#include "uiinfra/pixmapimage.h"

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
#include <qquickpainteditem.h>
#include <qquickstyle.h>
#include <qwidgetaction.h>

namespace opaq {
using namespace inf;

static const std::string s_unit(u8"µg/m³");

MapView::MapView(QWidget* parent)
: QWidget(parent)
, _colorMap("rdylgn_r")
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

            auto* object    = _ui.mapWidget->rootObject();
            _qmlMap         = object->findChild<QObject*>(QStringLiteral("map"));
            _qmlRaster      = object->findChild<QObject*>(QStringLiteral("raster"));
            _qmlRasterImage = object->findChild<ui::PixmapImage*>(QStringLiteral("rasterimage"));

            assert(_qmlMap);
            assert(_qmlRaster);
            assert(_qmlRasterImage);
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
    connect(this, &MapView::legendUpdated, this, &MapView::onUpdateLegend);

    _valueProvider.setUnit(s_unit);
    _valueProvider.setPrecision(2);
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

void MapView::clearData()
{
    if (_qmlRaster) {
        _qmlRaster->setProperty("source", "");
        _qmlRaster->setProperty("visible", false);
    }

    _legendModel.clear();
    _valueProvider.clearData();
    _pointSourceModel.clear();
}

void MapView::setData(const RasterPtr& data)
{
    setCursor(Qt::WaitCursor);

    JobRunner::queue([this, data]() {
        processRasterForDisplay(data);
    });
}

void MapView::setPointSourceData(std::vector<PointSourceModelData> data)
{
    _pointSourceModel.setModelData(std::move(data));
}

void MapView::setColorMap(std::string_view name)
{
    _colorMap = name;

    if (_warpedDataSource) {
        setCursor(Qt::WaitCursor);
        JobRunner::queue([this]() {
            applyColorMap();
        });
    }
}

void MapView::applyLegendSettings(const LegendSettings& settings)
{
    _legendSettings = settings;
}

void MapView::setVisibleRegion(const inf::GeoMetadata& meta)
{
    if (_qmlMap && meta.projection_epsg().has_value()) {
        _qmlMap->setProperty("visibleRegion", QVariant::fromValue(rasterViewPort(meta)));
    }
}

void MapView::setupQml()
{
    qmlRegisterType<ui::PixmapImage>("inf.ui.location", 1, 0, "PixmapImage");

    _ui.mapWidget->rootContext()->setContextProperty(QStringLiteral("valueprovider"), &_valueProvider);
    _ui.mapWidget->rootContext()->setContextProperty(QStringLiteral("legendmodel"), &_legendModel);
    _ui.mapWidget->rootContext()->setContextProperty(QStringLiteral("pointsourcemodel"), &_pointSourceModel);

    _ui.mapWidget->setSource(QUrl(QStringLiteral("qrc:/mapview.qml")));
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
        _warpedDataSource = std::make_shared<gdx::DenseRaster<double>>(gdx::warp_raster(*raster, 3857));
        applyColorMap();
    } catch (const std::exception& e) {
        Log::error("Failed to display raster data: {}", e.what());
        emit rasterOperationFailed();
    }
}

static QImage createRasterImage(const RasterDisplayData& data)
{
    if (!data.raster) {
        return QImage();
    }

    auto iter = std::max_element(data.legend.entries.begin(), data.legend.entries.end(), [](auto& lhs, auto& rhs) {
        return lhs.upperBound < rhs.upperBound;
    });

    auto maxValue = iter->upperBound;
    auto maxColor = iter->color;

    auto rgbaData = std::make_unique<std::vector<Color>>(data.raster->size());
    std::transform(optional_value_begin(*data.raster), optional_value_end(*data.raster), rgbaData->begin(), [&](const auto& value) {
        if (value.is_nodata() || (data.legend.zeroIsNodata && *value == 0)) {
            return Color{0, 0, 0, 0};
        }

        for (auto& entry : data.legend.entries) {
            if (*value >= entry.lowerBound && *value < entry.upperBound) {
                return entry.color;
            }
        }

        if (*value >= maxValue) {
            return maxColor;
        }

        return Color{0, 0, 0, 0};
    });

    auto width  = data.raster->cols();
    auto height = data.raster->rows();

    auto dataPtr = rgbaData.release();
    return QImage(
        reinterpret_cast<uint8_t*>(dataPtr->data()), width, height, QImage::Format_RGBA8888, [](void* data) {
            delete reinterpret_cast<std::vector<Color>*>(data);
        },
        dataPtr);
}

void MapView::onRasterDisplay(const RasterDisplayData& displayData, QPixmap pixmap)
{
    if (_qmlRaster && _qmlRasterImage) {
        _qmlRaster->setProperty("visible", true);
        _qmlRaster->setProperty("coordinate", QVariant::fromValue(displayData.coordinate));
        _qmlRaster->setProperty("zoomLevel", displayData.zoomLevel);
        _qmlRasterImage->setImage(pixmap);
    } else {
        Log::warn("Raster display failed. Qml error");
    }

    unsetCursor();
}

void MapView::onRasterOperationFailed()
{
    unsetCursor();
    clearData();
}

void MapView::onUpdateLegend(Legend legend)
{
    _pointSourceModel.setLegend(legend);
    _legendModel.setLegend(std::move(legend));
}

void MapView::applyColorMap()
{
    if (_warpedDataSource) {
        double rasterMin = 0, rasterMax = 100;

        if (!_legendSettings.minValue.has_value() || !_legendSettings.maxValue.has_value()) {
            std::tie(rasterMin, rasterMax) = gdx::minmax(*_warpedDataSource);
        }

        auto legendMin = _legendSettings.minValue.value_or(rasterMin);
        auto legendMax = _legendSettings.maxValue.value_or(rasterMax);

        auto legend = create_numeric_legend(legendMin, legendMax, _legendSettings.categories, _colorMap, LegendScaleType::Linear);
        generate_legend_names(legend, 2, s_unit);

        auto displayData   = createRasterDisplayData(_colorMap, _warpedDataSource);
        displayData.legend = legend;

        _valueProvider.setData(_warpedDataSource);
        emit legendUpdated(std::move(legend)); // decouple: models used in qml cannot be reset outside the ui thread
        emit rasterReadyForDisplay(displayData, QPixmap::fromImage(createRasterImage(displayData)));
    } else {
        auto legend = create_numeric_legend(_legendSettings.minValue.value_or(0.0), _legendSettings.maxValue.value_or(100), _legendSettings.categories, _colorMap, LegendScaleType::Linear);
        generate_legend_names(legend, 2, s_unit);
        emit legendUpdated(std::move(legend)); // decouple: models used in qml cannot be reset outside the ui thread
    }
}

}
