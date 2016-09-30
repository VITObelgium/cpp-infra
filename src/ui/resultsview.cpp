#include "resultsview.h"

#include "config/Component.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QVXYModelMapper>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>

namespace OPAQ
{

QT_CHARTS_USE_NAMESPACE

ResultsView::ResultsView(QWidget* parent)
: QWidget(parent)
, _model(this)
, _tableView(nullptr)
, _rows(0)
, _axisX(nullptr)
, _axisY(nullptr)
, _chart(nullptr)
{
    // create simple model for storing data
    // user's table data model

    // create table view and add model to it
    _tableView = new QTableView();
    _tableView->setModel(&_model);
    _tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    _chart = new QChart;
    _chart->setAnimationOptions(QChart::AllAnimations);

    auto* chartView = new QChartView(_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(640, 480);

    _axisX = new QValueAxis();
    _axisX->setRange(0, 4);
    _axisX->setLabelFormat("Day %d");
    _axisX->setTitleText("Forecast");
    _chart->addAxis(_axisX, Qt::AlignBottom);

    _axisY = new QValueAxis();
    _axisY->setRange(0, 100);
    _axisY->setTickCount(10);
    _axisY->setTitleText("Pollution");
    _chart->addAxis(_axisY, Qt::AlignLeft);

    // create main layout
    auto* mainLayout = new QGridLayout;
    mainLayout->addWidget(_tableView, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnMinimumWidth(0, 300);
    setLayout(mainLayout);
}

void ResultsView::setForecastHorizon(TimeInterval forecastHorizon)
{
    _rows = static_cast<int>(forecastHorizon.getDays()) + 1;
}

void ResultsView::setModels(const std::vector<Config::Component*>& models)
{
    int column = 0;
    for (auto* model : models)
    {
        QLineSeries* series = new QLineSeries();
        series->setName(model->name.c_str());
        auto* mapper = new QVXYModelMapper(this);

        mapper->setXColumn(static_cast<int>(models.size()));
        mapper->setYColumn(column);
        mapper->setSeries(series);
        mapper->setModel(&_model);
        _chart->addSeries(series);
        series->attachAxis(_axisX);
        series->attachAxis(_axisY);

        // get the color of the series and use it for showing the mapped area
        QString seriesColorHex = "#" + QString::number(series->pen().color().rgb(), 16).right(6).toUpper();
        _model.addMapping(seriesColorHex, QRect(column++, 0, 1, _rows));
    }
}

void ResultsView::updateResultsForStation(ForecastBuffer& buffer,
                                          DateTime baseTime,
                                          const std::string& stationName,
                                          TimeInterval forecastHorizon,
                                          const std::string& pollutantId,
                                          Aggregation::Type agg)
{
    _model.updateResults(buffer, baseTime, stationName, forecastHorizon, pollutantId, agg);
    _tableView->setColumnHidden(_model.rowCount() - 1, true);
}
}
