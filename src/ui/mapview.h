#pragma once

#include "imageprovider.h"
#include "infra/gdal.h"
#include "infra/point.h"
#include "typeregistrations.h"
#include "ui_mapview.h"
#include "uiinfra/rastervalueprovider.h"

#include <qgeorectangle.h>
#include <qmenu.h>
#include <qtoolbar.h>

QT_FORWARD_DECLARE_CLASS(QWidgetAction)
QT_FORWARD_DECLARE_CLASS(QAbstractListModel)

namespace opaq {

class MapView : public QWidget
{
    Q_OBJECT

public:
    MapView(QWidget* parent = nullptr);
    void setData(const RasterPtr& data);

signals:
    void mouseMoved(double, double, float);
    void rasterReadyForDisplay(RasterDataId id, const QGeoCoordinate& coord, double zoomLevel);
    void rasterOperationFailed();

private:
    void setupQml();

    void processRasterForDisplay(const RasterPtr& raster);
    void onRasterDisplay(RasterDataId raster, const QGeoCoordinate& coord, double zoomLevel);
    void onRasterOperationFailed();

    Ui::MapView _ui;
    QObject* _qmlMap    = nullptr;
    QObject* _qmlRaster = nullptr;

    uiinfra::RasterValueProvider<gdx::DenseRaster<double>> _valueProvider;

    ImageProviderRasterDataStorage _dataStorage;

    // Unwarped original raster data
    RasterPtr _originalDataSource;
};
}
