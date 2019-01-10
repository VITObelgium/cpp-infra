#include "opaqvalidation.h"
#include "ConfigurationHandler.h"
#include "Engine.h"
#include "config/Component.h"
#include "uiinfra/userinteraction.h"

#include "ui_opaqvalidation.h"

Q_DECLARE_METATYPE(opaq::Aggregation::Type)

namespace opaq {

using namespace inf;

OpaqValidation::OpaqValidation(QWidget* parent)
: QWidget(parent)
, _model(this)
{
    _ui.setupUi(this);

    _ui.fromDateEdit->setDate(QDate::currentDate());
    _ui.toDateEdit->setDate(QDate::currentDate());

    connect(_ui.validateButton, &QPushButton::clicked, [this]() {
        runValidation();
    });
}

void OpaqValidation::setConfig(ConfigurationHandler& config)
{
    _config = &config;
}

void OpaqValidation::setEngine(Engine& engine)
{
    _engine = &engine;
}

void OpaqValidation::setModels(const std::vector<config::Component>& models)
{
    for (auto& model : models) {
        auto* item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, model.name.c_str());
        item->setData(Qt::CheckStateRole, Qt::Checked);
        _ui.modelsListWidget->addItem(item);
    }

    //_ui.resultsView->setModels(_model, models);
}

void OpaqValidation::setStationModel(QAbstractItemModel& model)
{
    _ui.stationComboBox->setModel(&model);

    connect(&model, &QAbstractListModel::dataChanged, [this]() {
        _ui.stationComboBox->setCurrentIndex(0);
    });
}

void OpaqValidation::setPollutantModel(QAbstractItemModel& model)
{
    _ui.pollutantComboBox->setModel(&model);

    connect(&model, &QAbstractListModel::dataChanged, [this]() {
        _ui.pollutantComboBox->setCurrentIndex(0);
    });
}

void OpaqValidation::setAggregationModel(QAbstractItemModel& model)
{
    _ui.aggregationComboBox->setModel(&model);
}

std::string OpaqValidation::pollutant() const noexcept
{
    return _ui.pollutantComboBox->currentData(Qt::UserRole).value<QString>().toStdString();
}

Aggregation::Type OpaqValidation::aggregation() const noexcept
{
    assert(_ui.aggregationComboBox->currentData(Qt::UserRole).canConvert<Aggregation::Type>());
    return _ui.aggregationComboBox->currentData(Qt::UserRole).value<Aggregation::Type>();
}

std::string OpaqValidation::station() const noexcept
{
    return _ui.stationComboBox->currentData(Qt::UserRole).value<QString>().toStdString();
}

chrono::date_time OpaqValidation::startTime() const noexcept
{
    return chrono::from_date_string(_ui.fromDateEdit->date().toString("yyyy-MM-dd").toStdString());
}

chrono::date_time OpaqValidation::endTime() const noexcept
{
    return chrono::from_date_string(_ui.toDateEdit->date().toString("yyyy-MM-dd").toStdString());
}

chrono::days OpaqValidation::forecastHorizon() const noexcept
{
    return chrono::days(_ui.forecastHorizonComboBox->currentIndex() + 1);
}

void OpaqValidation::runValidation()
{
    assert(_config);
    assert(_engine);

    auto pol = pollutant();
    _config->getOpaqRun().setPollutantName(pollutant(), Aggregation::getName(aggregation()));

    try {
        _model.clear();
        for (int i = 0; i < _ui.modelsListWidget->count(); ++i) {
            auto* model = _ui.modelsListWidget->item(i);
            if (model->checkState() == Qt::CheckState::Checked) {
                auto results = _engine->validate(_config->getOpaqRun(), forecastHorizon(), station(), startTime(), endTime(), model->text().toStdString());
                _model.addResult(model->text().toStdString(), std::move(results));
            }
        }

        _ui.scatterView->setModel(_model);
        _ui.lineView->setModel(_model);
    } catch (const opaq::ParseException& e) {
        ui::displayError(tr("Failed to parse date"), e.what());
    } catch (const std::exception& e) {
        ui::displayError(tr("Failed to run validation"), e.what());
    }
}

}
