#include <QApplication>
#include <QtPlugin>

#include "infra/log.h"
#include "mainwindow.h"
#include "opaqconfig.h"
#include "uiinfra/application.h"
#include "uiinfra/logsinkmodel.h"

#ifdef STATIC_QT
#ifdef WIN32
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(GeoServiceProviderFactoryEsri);

Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtLocationDeclarativeModule)
Q_IMPORT_PLUGIN(QtPositioningDeclarativeModule)
#endif
#endif

using namespace inf;

int main(int argc, char* argv[])
{
    //Q_INIT_RESOURCE(application);

#ifndef STATIC_QT
    QCoreApplication::addLibraryPath("./plugins");
#endif

    auto logSinkModel = std::make_shared<uiinfra::LogSinkModelMt>();
    Log::add_custom_sink(logSinkModel);
    LogRegistration logging("opaq");

#ifndef NDEBUG
    Log::setLevel(Log::Level::Debug);
#endif

    uiinfra::Application app(argc, argv);
    QCoreApplication::setOrganizationName("VITO");
    QCoreApplication::setOrganizationDomain("vito.be");
    QCoreApplication::setApplicationName("Opaq");
    QCoreApplication::setApplicationVersion(OPAQ_VERSION);

    opaq::MainWindow mainWin(logSinkModel);
    mainWin.setMinimumSize(640, 480);
    mainWin.show();
    return app.exec();
}
