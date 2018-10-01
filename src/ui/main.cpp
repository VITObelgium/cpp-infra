#include <QApplication>
#include <QtPlugin>

#include "infra/log.h"
#include "mainwindow.h"
#include "opaqconfig.h"

#ifdef STATIC_QT
#if defined WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif
#endif

using namespace inf;

int main(int argc, char* argv[])
{
    //Q_INIT_RESOURCE(application);

#ifndef STATIC_QT
    QCoreApplication::addLibraryPath("./plugins");
#endif

    Log::add_console_sink(Log::Colored::On);
    LogRegistration logging("opaq");

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("VITO");
    QCoreApplication::setOrganizationDomain("vito.be");
    QCoreApplication::setApplicationName("Opaq");
    QCoreApplication::setApplicationVersion(OPAQ_VERSION);

    opaq::MainWindow mainWin;
    mainWin.setMinimumSize(640, 480);
    mainWin.show();
    return app.exec();
}
