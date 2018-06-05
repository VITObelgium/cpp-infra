#include <QApplication>
#include <QtPlugin>

#include "config.h"
#include "mainwindow.h"

#ifdef STATIC_QT
#if defined WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif
#endif

int main(int argc, char* argv[])
{
    //Q_INIT_RESOURCE(application);

#ifndef STATIC_QT
    QCoreApplication::addLibraryPath("./plugins");
#endif

    Log::initLogger("");
    Logger logger("main");

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
