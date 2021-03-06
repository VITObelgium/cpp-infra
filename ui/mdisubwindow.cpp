#include "uiinfra/mdisubwindow.h"

#include <qlayout.h>
#include <qsettings.h>

namespace inf::ui {

MdiSubWindow::MdiSubWindow(QWidget* internalWidget, QWidget* parent)
: QMdiSubWindow(parent)
{
    setWidget(internalWidget);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(900, 480);
    restoreSettings();

    connect(this, &QMdiSubWindow::windowStateChanged, this, &MdiSubWindow::onMdiWindowStateChange);
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

    if (windowState() & Qt::WindowMaximized) {
        setLayoutMargin(0, 0, 0, 0);
    }

    repaint();
}

void MdiSubWindow::onMdiWindowStateChange(Qt::WindowStates oldState, Qt::WindowStates newState)
{
    // Set the margin of the MDI Widget layout to 0 when it is maximized to better align with the
    // other UI elements

    if (!(oldState & Qt::WindowMaximized) && (newState & Qt::WindowMaximized)) {
        int top, left, right, bottom;
        widget()->layout()->getContentsMargins(&top, &left, &right, &bottom);
        _defaultMarginTop    = top;
        _defaultMarginLeft   = left;
        _defaultMarginRight  = right;
        _defaultMarginBottom = bottom;
        setLayoutMargin(0, 0, 0, 0);
    } else if ((oldState & Qt::WindowMaximized) && !(newState & Qt::WindowMaximized)) {
        if (_defaultMarginTop.has_value()) {
            setLayoutMargin(*_defaultMarginTop, *_defaultMarginLeft, *_defaultMarginRight, *_defaultMarginBottom);
        }
    }
}

void MdiSubWindow::setLayoutMargin(int top, int left, int right, int bottom)
{
    widget()->layout()->setContentsMargins(top, left, right, bottom);
}
}
