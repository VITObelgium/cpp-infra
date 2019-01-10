#pragma once

#include "imageprovider.h"
#include "infra/gdal.h"
#include "infra/legend.h"
#include "infra/point.h"
#include "typeregistrations.h"
#include "ui_mapview.h"
#include "uiinfra/legendmodel.h"
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
    void setColorMap(std::string_view name);

private:
signals:
    void rasterReadyForDisplay(RasterDataId id, const QGeoCoordinate& coord, double zoomLevel);
    void rasterOperationFailed();
    void legendUpdated(inf::Legend legend);

private:
    void setupQml();

    void processRasterForDisplay(const RasterPtr& raster);
    void onRasterDisplay(RasterDataId raster, const QGeoCoordinate& coord, double zoomLevel);
    void onRasterOperationFailed();
    void onUpdateLegend(inf::Legend legend);

    void applyColorMap();

    Ui::MapView _ui;
    QObject* _qmlMap    = nullptr;
    QObject* _qmlRaster = nullptr;
    QObject* _qmlLegend = nullptr;

    inf::ui::RasterValueProvider<gdx::DenseRaster<double>> _valueProvider;
    inf::ui::LegendModel _legendModel;

    ImageProviderRasterDataStorage _dataStorage;

    RasterPtr _warpedDataSource;

    std::string _colorMap;
};
}
