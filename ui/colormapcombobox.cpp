#include "uiinfra/colormapcombobox.h"

#include <cassert>

namespace inf::ui {

ColorMapComboBox::ColorMapComboBox(QWidget* parent)
: DelegateComboBox(parent)
{
    setItemDelegate(&_delegate);
}

}
