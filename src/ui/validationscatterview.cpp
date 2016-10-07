#include "validationscatterview.h"

#include "config/Component.h"
#include "validationresultsmodel.h"

#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QHXYModelMapper>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>

namespace OPAQ
{

QT_CHARTS_USE_NAMESPACE

ValidationScatterView::ValidationScatterView(QWidget* parent)
: QWidget(parent)
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
    _axisX->setRange(0, 75);
    _axisX->setTitleText("Observations");
    _chart->addAxis(_axisX, Qt::AlignBottom);

    _axisY = new QValueAxis();
    _axisY->setRange(0, 75);
    _axisY->setTitleText("OPAQ - RTC");
    _chart->addAxis(_axisY, Qt::AlignLeft);

    // create main layout
    auto* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(chartView);
    setLayout(mainLayout);
}

void ValidationScatterView::setModel(ValidationResultsModel& model)
{
    _chart->removeAllSeries();

    auto rowCount = model.rowCount();
    for (int i = 0; i < rowCount; i+=2)
    {
        auto* series = new QScatterSeries();
        series->setMarkerSize(5.0);
        series->setName(model.headerData(i, Qt::Orientation::Horizontal, Qt::DisplayRole).value<QString>());
        auto* mapper = new QHXYModelMapper(this);

        mapper->setXRow(i);
        mapper->setYRow(i+1);
        mapper->setSeries(series);
        mapper->setModel(&model);
        _chart->addSeries(series);
        series->attachAxis(_axisX);
        series->attachAxis(_axisY);
    }
}

}
