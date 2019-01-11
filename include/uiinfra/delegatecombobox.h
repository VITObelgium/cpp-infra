#pragma once

#include <qcombobox.h>

namespace inf::ui {

/*! Combobox that uses the item delegate to paint the current item
 */
class DelegateComboBox : public QComboBox
{
    Q_OBJECT

public:
    DelegateComboBox(QWidget* widget = nullptr);

    void paintEvent(QPaintEvent* e) override;
};
}
