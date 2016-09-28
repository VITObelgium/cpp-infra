#include <QApplication>
// #include <QCommandLineParser>
// #include <QCommandLineOption>
#include <QtPlugin>

#include "config.h"
#include "mainwindow.h"

#if defined WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined __APPLE__
    Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif

int main(int argc, char* argv[])
{
    //Q_INIT_RESOURCE(application);

    Log::initLogger("");

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("VITO");
    QCoreApplication::setApplicationName("Opaq");
    QCoreApplication::setApplicationVersion(OPAQ_VERSION);

    // QCommandLineParser parser;
    // parser.setApplicationDescription(QCoreApplication::applicationName());
    // parser.addHelpOption();
    // parser.addVersionOption();
    // parser.addPositionalArgument("file", "The file to open.");
    // parser.process(app);

    OPAQ::MainWindow mainWin;
    // if (!parser.positionalArguments().isEmpty())
    //     mainWin.loadFile(parser.positionalArguments().first());
    mainWin.setFixedSize(640, 480);
    mainWin.show();
    return app.exec();
}