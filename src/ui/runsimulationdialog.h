#pragma once

#include "Aggregation.h"

#include <vector>
#include <memory>
#include <QDialog>

namespace Ui
{
class RunSimulationDialog;
}

namespace OPAQ
{

class Pollutant;

class RunSimulationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunSimulationDialog(const std::vector<Pollutant>& pollutants, QWidget* parent = 0);
    ~RunSimulationDialog();

    std::string pollutant() const noexcept;
    Aggregation::Type aggregation() const noexcept;
    std::string basetime() const noexcept;

private:
    std::unique_ptr<Ui::RunSimulationDialog> ui;
};

}
