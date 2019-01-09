#include "uiinfra/colorcombobox.h"

#include <cassert>

namespace inf::ui {

ColorComboBox::ColorComboBox(QWidget* parent)
: DelegateComboBox(parent)
{
    setItemDelegate(&_delegate);
}

void ColorComboBox::setModelColorRole(int role)
{
    _delegate.setRole(role);
}
int ColorComboBox::colorIndex(const QColor& color) const
{
    const int items = model()->rowCount();

    for (int i = 0; i < items; ++i) {
        if (model()->index(i, 0).data(_delegate.role()).value<QColor>() == color) {
            return i;
        }
    }

    return -1;
}
}
