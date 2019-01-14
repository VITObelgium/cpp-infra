#include "infra/gdal.h"
#include "infra/log.h"
#include "jobrunner.h"
#include "mainwindow.h"
#include "opaqconfig.h"
#include "uiinfra/application.h"
#include "uiinfra/logsinkmodel.h"

#include <QApplication>
#include <QtGlobal>
#include <QtPlugin>
#include <qmessagebox.h>

#ifdef STATIC_QT
Q_IMPORT_PLUGIN(QJpegPlugin);
Q_IMPORT_PLUGIN(GeoServiceProviderFactoryEsri);
Q_IMPORT_PLUGIN(QGeoServiceProviderFactoryOsm);
Q_IMPORT_PLUGIN(QGeoServiceProviderFactoryItemsOverlay);

Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtQuickLayoutsPlugin)
Q_IMPORT_PLUGIN(QtQuickControls2Plugin)
Q_IMPORT_PLUGIN(QtQuickControls2FusionStylePlugin)
Q_IMPORT_PLUGIN(QtQuick2WindowPlugin)
Q_IMPORT_PLUGIN(QtQuickTemplates2Plugin)
Q_IMPORT_PLUGIN(QtLocationDeclarativeModule)
Q_IMPORT_PLUGIN(QtPositioningDeclarativeModule)

#ifdef Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

#ifdef Q_OS_MACOS
Q_IMPORT_PLUGIN(QSQLiteDriverPlugin);
Q_IMPORT_PLUGIN(QMacStylePlugin);
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
Q_IMPORT_PLUGIN(QGeoServiceProviderFactoryMapboxGL);
#endif
#endif

using namespace inf;

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(qmlcontrols);

#ifndef STATIC_QT
    QCoreApplication::addLibraryPath("./plugins");
#endif

    auto logSinkModel = std::make_shared<uiinfra::LogSinkModelMt>();
    Log::add_custom_sink(logSinkModel);
    LogRegistration logging("opaq");

#ifdef NDEBUG
    Log::setLevel(Log::Level::Info);
#else
    Log::setLevel(Log::Level::Debug);
#endif

    try {
        opaq::JobRunner::setUncaughtExceptionCb(nullptr, [](const std::exception_ptr& e) {
            try {
                std::rethrow_exception(e);
            } catch (const std::exception& e) {
                Log::critical("Uncaught exception in job runner ({})", e.what());
            }
        });

        gdal::Registration gdalReg;
        uiinfra::Application app(argc, argv);
        QCoreApplication::setOrganizationName("VITO");
        QCoreApplication::setOrganizationDomain("vito.be");
        QCoreApplication::setApplicationName("Opaq");
        QCoreApplication::setApplicationVersion(OPAQ_VERSION);

        opaq::JobRunner::start(1);

        opaq::MainWindow mainWin(logSinkModel);
        mainWin.setMinimumSize(640, 480);
        mainWin.show();
        int rc = app.exec();
        opaq::JobRunner::stop();
        return rc;
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), e.what(), QMessageBox::Ok);
        opaq::JobRunner::stop();
        return EXIT_FAILURE;
    }
}
