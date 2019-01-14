#pragma once

#include "legendsettings.h"
#include "ui_preferencesdialog.h"

#include <qdialog.h>
#include <set>

namespace opaq {

class PreferencesDialog : public QDialog, private Ui::PreferencesDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget* parent = nullptr);

    void saveSettings();
    LegendSettings legendSettings() const;

private:
    void restoreSettings();
};
}
