#pragma once

//#include "gdx/denseraster.h"
//#include "infra/gdal.h"
#include "infra/point.h"
#include "ui_mapview.h"

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

signals:
    void mouseMoved(double, double, float);
    //void rasterReadyForDisplay(RasterDataId id, const QGeoCoordinate& coord, double zoomLevel);
    //void rasterOperationFailed();

private:
    void setupQml();

    /*void processRasterForDisplay(const std::shared_ptr<gdx::DenseRaster<float>>& raster);
    void onRasterDisplay(RasterDataId raster, const QGeoCoordinate& coord, double zoomLevel);
    void onRasterOperationFailed();
    void onSaveMap();*/

    Ui::MapView _ui;
    QObject* _qmlMap         = nullptr;
    QObject* _qmlRaster      = nullptr;
    QObject* _qmlRasterImage = nullptr;

    //inf::gdal::CoordinateTransformer _transformer;
    //inf::Point<double> _previousCoordinate;

    // Unwarped original raster data
    //std::shared_ptr<gdx::DenseRaster<float>> _originalRasterSource;
    // Unwarped displayed raster data
    //std::shared_ptr<gdx::DenseRaster<float>> _currentRasterSource;
    // The name of the raster data being displayed
    //std::string _currentRasterName;
};
}
