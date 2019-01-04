#include "mapview.h"

#include "infra/log.h"

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

            auto* object    = _ui.mapWidget->rootObject();
            _qmlMap         = object->findChild<QObject*>(QStringLiteral("map"));
            _qmlRaster      = object->findChild<QObject*>(QStringLiteral("raster"));
            _qmlRasterImage = object->findChild<QObject*>(QStringLiteral("rasterimage"));
            assert(_qmlMap);
            assert(_qmlRaster);
            assert(_qmlRasterImage);

            //QObject::connect(_qmlMap, SIGNAL(mouseMoved(QVariant)), this, SLOT(onMouseMoveEvent(QVariant)));
        }
    });

    setupQml();

    //connect(this, &MapView::rasterReadyForDisplay, this, &MapView::onRasterDisplay);
    //connect(this, &MapView::rasterOperationFailed, this, &MapView::onRasterOperationFailed);
}

void MapView::setupQml()
{
    _ui.mapWidget->setSource(QUrl(QStringLiteral("qrc:/mapview.qml")));
}

//// Calculate the zoom level where one pixel is equal to the cellsize at latitude 0.0
//static double calculateZoomLevel(double cellSize)
//{
//    // source: https://msdn.microsoft.com/en-us/library/bb259689.aspx
//    // cellSize = std::cos(latitude * M_PI / 180) * 2 * M_PI * 6378137) / (256 * 2 ^ zoomLevel);
//
//    return std::log2((std::cos(0.0) * 2.0 * M_PI * 6378137.0) / cellSize / 256.0);
//}
}
