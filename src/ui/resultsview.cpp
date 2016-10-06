#include "resultsview.h"

#include "config/Component.h"
#include "stationresultsmodel.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QHXYModelMapper>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>

namespace OPAQ
{

QT_CHARTS_USE_NAMESPACE

ResultsView::ResultsView(QWidget* parent)
: QWidget(parent)
, _rows(0)
, _axisX(nullptr)
, _axisY(nullptr)
, _chart(nullptr)
{
    // create simple model for storing data
    // user's table data model

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
    auto* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(chartView);
    setLayout(mainLayout);
}

void ResultsView::setForecastHorizon(TimeInterval forecastHorizon)
{
    _rows = static_cast<int>(forecastHorizon.getDays()) + 1;
}

void ResultsView::setModels(StationResultsModel& model, const std::vector<Config::Component>& modelComponents)
{
    int row = 1;
    for (auto& comp : modelComponents)
    {
        QLineSeries* series = new QLineSeries();
        series->setName(comp.name.c_str());
        auto* mapper = new QHXYModelMapper(this);

        mapper->setXRow(0);
        mapper->setYRow(row);
        mapper->setSeries(series);
        mapper->setModel(&model);
        _chart->addSeries(series);
        series->attachAxis(_axisX);
        series->attachAxis(_axisY);

        // get the color of the series and use it for showing the mapped area
        QString seriesColorHex = "#" + QString::number(series->pen().color().rgb(), 16).right(6).toUpper();
        model.addMapping(seriesColorHex, QRect(0, row++, _rows, 1));
    }
}

}
