#pragma once

#include <qmdisubwindow.h>

namespace inf::ui {

class MdiSubWindow : public QMdiSubWindow
{
public:
    MdiSubWindow(QWidget* internalWidget, QWidget* parent = nullptr);
    virtual ~MdiSubWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void onMdiWindowStateChange(Qt::WindowStates oldState, Qt::WindowStates newState);
    void setLayoutMargin(int margin);

    void saveSettings();
    void restoreSettings();
};
}
