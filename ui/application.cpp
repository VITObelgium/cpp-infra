#include "uiinfra/application.h"
#include "infra/log.h"

namespace inf::ui {

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
        inf::Log::debug("[Qt {}] {} ({}:{}.{})", levelString(type), msg.toStdString(), context.file, context.line, context.function);
    } else {
        inf::Log::debug("[Qt {}] {}", levelString(type), msg.toStdString());
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
        inf::Log::critical("Uncaught exception: {}", e.what());
    }

    return false;
}
}
