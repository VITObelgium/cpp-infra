#include <QApplication>
#include <QtPlugin>

#include "infra/log.h"
#include "mainwindow.h"
#include "opaqconfig.h"
#include "uiinfra/application.h"
#include "uiinfra/logsinkmodel.h"

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

    auto logSinkModel = std::make_shared<uiinfra::LogSinkModelMt>();
    Log::add_custom_sink(logSinkModel);
    LogRegistration logging("opaq");

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
