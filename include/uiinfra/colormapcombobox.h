#pragma once

#include "uiinfra/coloredrectangledelegate.h"
#include "uiinfra/delegatecombobox.h"
#include "uiinfra/gradientdelegate.h"

#include <qcombobox.h>

namespace inf::ui {

/*! Combobox for selecting a color map
 */
class ColorMapComboBox : public DelegateComboBox
{
    Q_OBJECT

public:
    ColorMapComboBox(QWidget* parent = nullptr);

private:
    GradientDelegate _delegate;
};

}
