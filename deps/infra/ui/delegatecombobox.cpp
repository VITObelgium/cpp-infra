#include "uiinfra/delegatecombobox.h"

#include <cassert>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qlistwidget.h>
#include <qstylepainter.h>

namespace uiinfra {

DelegateComboBox::DelegateComboBox(QWidget* widget)
: QComboBox(widget)
{
}

void DelegateComboBox::paintEvent(QPaintEvent* e)
{
    if (auto* delegate = itemDelegate(); delegate != nullptr) {
        QStylePainter painter(this);
        painter.setPen(palette().color(QPalette::Text));

        // draw the combobox frame, focusrect and selected etc.
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        painter.drawComplexControl(QStyle::CC_ComboBox, opt);

        QStyleOptionViewItem optview;
        optview.rect = opt.rect.adjusted(1, 1, -20, -1); //compensate for frame and arrow
        delegate->paint(&painter, optview, model()->index(currentIndex(), modelColumn()));
    } else {
        QComboBox::paintEvent(e);
    }
}
}
