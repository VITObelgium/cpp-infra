#pragma once

#include "infra/gdal.h"
#include "infra/legend.h"
#include "infra/point.h"
#include "legendsettings.h"
#include "pointsourcemodel.h"
#include "typeregistrations.h"
#include "ui_mapview.h"
#include "uiinfra/legendmodel.h"
#include "uiinfra/rastervalueprovider.h"

#include <qgeorectangle.h>
#include <qmenu.h>
#include <qtoolbar.h>

QT_FORWARD_DECLARE_CLASS(QWidgetAction)
QT_FORWARD_DECLARE_CLASS(QAbstractListModel)

namespace inf::ui {
class PixmapImage;
}

namespace opaq {

struct RasterDisplayData;

class MapView : public QWidget
{
    Q_OBJECT

public:
    MapView(QWidget* parent = nullptr);
    void clearData();
    void setData(const RasterPtr& data);
    void setPointSourceData(std::vector<PointSourceModelData> data);
    void setColorMap(std::string_view name);
    void applyLegendSettings(const LegendSettings& settings);
    void setVisibleRegion(const inf::GeoMetadata& meta);

private:
signals:
    void rasterReadyForDisplay(const RasterDisplayData& displayData, QPixmap image);
    void rasterOperationFailed();
    void legendUpdated(inf::Legend legend);

private:
    void setupQml();

    void processRasterForDisplay(const RasterPtr& raster);
    void onRasterDisplay(const RasterDisplayData& displayData, QPixmap image);
    void onRasterOperationFailed();
    void onUpdateLegend(inf::Legend legend);

    void applyColorMap();

    Ui::MapView _ui;
    QObject* _qmlMap                      = nullptr;
    QObject* _qmlRaster                   = nullptr;
    inf::ui::PixmapImage* _qmlRasterImage = nullptr;

    inf::ui::RasterValueProvider<gdx::DenseRaster<double>> _valueProvider;
    inf::ui::LegendModel _legendModel;

    RasterPtr _warpedDataSource;

    std::string _colorMap;
    LegendSettings _legendSettings;
    PointSourceModel _pointSourceModel;
};
}
