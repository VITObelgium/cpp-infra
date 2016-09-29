#include <QApplication>
#include <QtPlugin>

#include "config.h"
#include "mainwindow.h"

#if defined WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

int main(int argc, char* argv[])
{
    //Q_INIT_RESOURCE(application);

    Log::initLogger("");

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("VITO");
    QCoreApplication::setApplicationName("Opaq");
    QCoreApplication::setApplicationVersion(OPAQ_VERSION);

    OPAQ::MainWindow mainWin;
    mainWin.setMinimumSize(640, 480);
    mainWin.show();
    return app.exec();
}