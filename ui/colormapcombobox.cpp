#include "uiinfra/colormapcombobox.h"

#include <cassert>

namespace uiinfra {

ColorMapComboBox::ColorMapComboBox(QWidget* parent)
: DelegateComboBox(parent)
{
    setItemDelegate(&_delegate);
}

}
