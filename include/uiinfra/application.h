#pragma once

#include <qapplication.h>

namespace uiinfra {

//! Application implementation that handles uncaugh exceptions.
class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int& argc, char* argv[]);

    bool notify(QObject* receiver, QEvent* event) override;
};
}
