#include "runsimulationdialog.h"
#include "ui_runsimulationdialog.h"

#include "Pollutant.h"

#include <QList>

namespace OPAQ
{

Q_DECLARE_METATYPE(Aggregation::Type)

RunSimulationDialog::RunSimulationDialog(const std::vector<Pollutant>& pollutants, QWidget* parent)
: QDialog(parent)
, ui(std::make_unique<Ui::RunSimulationDialog>())
{
    ui->setupUi(this);

    for (auto& pol : pollutants)
    {
        ui->pollutantComboBox->addItem(pol.getDescription().c_str(), QVariant(QString(pol.getName().c_str())));
    }

    ui->aggregateComboBox->addItem(Aggregation::getDisplayName(Aggregation::DayAvg).c_str(), Aggregation::DayAvg);
    ui->aggregateComboBox->addItem(Aggregation::getDisplayName(Aggregation::Max1h).c_str(), Aggregation::Max1h);
    ui->aggregateComboBox->addItem(Aggregation::getDisplayName(Aggregation::Max8h).c_str(), Aggregation::Max8h);

    ui->baseTimeDateEdit->setDate(QDate(2016, 3, 2));
}

RunSimulationDialog::~RunSimulationDialog() = default;

std::string RunSimulationDialog::pollutant() const noexcept
{
    return ui->pollutantComboBox->currentData().value<QString>().toStdString();
}

Aggregation::Type RunSimulationDialog::aggregation() const noexcept
{
    assert(ui->aggregateComboBox->currentData().canConvert<Aggregation::Type>());
    return ui->aggregateComboBox->currentData().value<Aggregation::Type>();
}

std::string RunSimulationDialog::basetime() const noexcept
{
    return ui->baseTimeDateEdit->date().toString("yyyy-MM-dd").toStdString();
}

}