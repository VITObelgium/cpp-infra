#include "uiinfra/mdisubwindow.h"

#include <qsettings.h>

namespace uiinfra {

MdiSubWindow::MdiSubWindow(QWidget* internalWidget, QWidget* parent)
: QMdiSubWindow(parent)
{
    setWidget(internalWidget);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(900, 480);
    restoreSettings();
}

MdiSubWindow::~MdiSubWindow()
{
    saveSettings();
}

void MdiSubWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QMdiSubWindow::closeEvent(event);
}

void MdiSubWindow::saveSettings()
{
    QSettings settings;
    settings.setValue(QString("%1/geometry").arg(widget()->objectName()), saveGeometry());
}

void MdiSubWindow::restoreSettings()
{
    QSettings settings;
    restoreGeometry(settings.value(QString("%1/geometry").arg(widget()->objectName())).toByteArray());
}
}
