#pragma once

#include <qapplication.h>

namespace inf::ui {

//! Application implementation that handles uncaught exceptions
// and routes the qt logging to the infra logging
class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int& argc, char* argv[]);

    bool notify(QObject* receiver, QEvent* event) override;
};
}
