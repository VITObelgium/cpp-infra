#include "uiinfra/application.h"
#include "infra/log.h"

#include <qpalette.h>

namespace inf::ui {

static void loggedMessageOutput(QtMsgType /*type*/, const QMessageLogContext& context, const QString& msg)
{
    if (context.file && context.line && context.function) {
        inf::Log::debug("[Qt] {} ({}:{}.{})", msg.toStdString(), context.file, context.line, context.function);
    } else {
        inf::Log::debug("[Qt] {}", msg.toStdString());
    }
}

Application::Application(int& argc, char* argv[])
: QApplication(argc, argv)
{
    qInstallMessageHandler(loggedMessageOutput);

#ifdef Q_OS_OSX
    // workaround for QTBUG-75321 (https://bugreports.qt.io/browse/QTBUG-75321)
    auto pal = palette();
    pal.setColor(QPalette::ButtonText, pal.color(QPalette::WindowText));
    setPalette(pal);
#endif
}

bool Application::notify(QObject* receiver, QEvent* event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch (std::exception& e) {
        inf::Log::critical("Uncaught exception: {}", e.what());
    }

    return false;
}
}
