#pragma once

#include <qmdisubwindow.h>

namespace uiinfra {

class MdiSubWindow : public QMdiSubWindow
{
public:
    MdiSubWindow(QWidget* internalWidget, QWidget* parent = nullptr);
    virtual ~MdiSubWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void saveSettings();
    void restoreSettings();
};
}
