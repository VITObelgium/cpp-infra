#include "preferencesdialog.h"
#include "infra/log.h"

#include <qsettings.h>

namespace opaq {

PreferencesDialog::PreferencesDialog(QWidget* parent)
: QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi(this);

    restoreSettings();
    //updateButtonAvailability();
}

void PreferencesDialog::saveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("mapsettings/legendcategories"), categoriesSpinBox->value());
    settings.setValue(QStringLiteral("mapsettings/legendminvalue"), minValueSpinBox->value());
    settings.setValue(QStringLiteral("mapsettings/legendmaxvalue"), maxValueSpinBox->value());
    settings.setValue(QStringLiteral("mapsettings/legendusedataminvalue"), useDataMinimumCheckBox->isChecked());
    settings.setValue(QStringLiteral("mapsettings/legendusedatamaxvalue"), useDataMaximumCheckBox->isChecked());
}

LegendSettings PreferencesDialog::legendSettings() const
{
    LegendSettings settings;
    settings.categories = categoriesSpinBox->value();

    if (!useDataMinimumCheckBox->isChecked()) {
        settings.minValue = minValueSpinBox->value();
    }

    if (!useDataMaximumCheckBox->isChecked()) {
        settings.maxValue = maxValueSpinBox->value();
    }

    return settings;
}

void PreferencesDialog::restoreSettings()
{
    QSettings settings;
    categoriesSpinBox->setValue(settings.value(QStringLiteral("mapsettings/legendcategories"), 10).toInt());
    minValueSpinBox->setValue(settings.value(QStringLiteral("mapsettings/legendminvalue"), 0.0).toDouble());
    maxValueSpinBox->setValue(settings.value(QStringLiteral("mapsettings/legendmaxvalue"), 100.0).toDouble());
    useDataMinimumCheckBox->setChecked(settings.value(QStringLiteral("mapsettings/legendusedataminvalue"), false).toBool());
    useDataMaximumCheckBox->setChecked(settings.value(QStringLiteral("mapsettings/legendusedatamaxvalue"), false).toBool());
}

}
