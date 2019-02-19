#pragma once

#include "uiinfra/coloredrectangledelegate.h"
#include "uiinfra/delegatecombobox.h"
#include "uiinfra/gradientdelegate.h"

#include <qcombobox.h>
namespace inf::ui {

/*! Combobox for selecting colors
 */
class ColorComboBox : public DelegateComboBox
{
    Q_OBJECT

public:
    ColorComboBox(QWidget* parent = nullptr);

    void setModelColorRole(int role);
    int colorIndex(const QColor& color) const;

private:
    ColoredRectangleDelegate _delegate;
};
}
