#include "uiinfra/application.h"
#include "infra/log.h"

namespace uiinfra {

Application::Application(int& argc, char* argv[])
: QApplication(argc, argv)
{
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
