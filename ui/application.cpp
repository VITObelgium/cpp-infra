#include "uiinfra/application.h"
#include "infra/log.h"

namespace uiinfra {

static const char* levelString(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "Debug";
    case QtInfoMsg:
        return "Info";
    case QtWarningMsg:
        return "Warning";
    case QtCriticalMsg:
        return "Critical";
    case QtFatalMsg:
        return "Fatal";
    }

    return "";
}

static void loggedMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (context.file && context.line && context.function) {
        infra::Log::debug("[Qt {}] {} ({}:{}.{})", levelString(type), msg.toStdString(), context.file, context.line, context.function);
    } else {
        infra::Log::debug("[Qt {}] {}", levelString(type), msg.toStdString());
    }
}

Application::Application(int& argc, char* argv[])
: QApplication(argc, argv)
{
    qInstallMessageHandler(loggedMessageOutput);
}

bool Application::notify(QObject* receiver, QEvent* event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch (std::exception& e) {
        infra::Log::critical("Uncaught exception: {}", e.what());
    }

    return false;
}
}
