#include "validationscatterview.h"

#include "config/Component.h"
#include "validationresultsmodel.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QHXYModelMapper>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>

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
    _chart->setAnimationOptions(QChart::SeriesAnimations);

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

    // Add reference lines
    addReferenceOriginLine(500, 1000, Qt::PenStyle::DashLine, QColor(Qt::GlobalColor::black)); //   2x
    addReferenceOriginLine(500, 500, Qt::PenStyle::SolidLine, QColor(Qt::GlobalColor::red));  //    x
    addReferenceOriginLine(500, 250, Qt::PenStyle::DashLine, QColor(Qt::GlobalColor::black));  // 0.5x

    for (auto* marker : _chart->legend()->markers())
    {
        marker->setVisible(false);
    }

    // create main layout
    auto* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(chartView);
    setLayout(mainLayout);
}

void ValidationScatterView::setModel(ValidationResultsModel& model)
{
    for (auto* series : _modelSeries)
    {
        _chart->removeSeries(series);
    }

    _modelSeries.clear();

    auto rowCount = model.rowCount() - 1;
    for (int i = 1; i < rowCount; i += 2)
    {
        auto* series = new QScatterSeries();
        series->setMarkerSize(5.0);
        series->setName(model.headerData(i, Qt::Orientation::Horizontal, Qt::DisplayRole).value<QString>());
        auto* mapper = new QHXYModelMapper(this);

        mapper->setXRow(i);
        mapper->setYRow(i + 1);
        mapper->setSeries(series);
        mapper->setModel(&model);
        _chart->addSeries(series);
        series->attachAxis(_axisX);
        series->attachAxis(_axisY);

        _modelSeries.push_back(series);
    }

    for (auto* marker : _chart->legend()->markers())
    {
        // Disconnect possible existing connection to avoid multiple connections
        disconnect(marker, &QLegendMarker::clicked, this, &ValidationScatterView::handleMarkerClicked);
        connect(marker, &QLegendMarker::clicked, this, &ValidationScatterView::handleMarkerClicked);
    }
}

void ValidationScatterView::handleMarkerClicked()
{
    QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
    Q_ASSERT(marker);

    switch (marker->type())
    {
    case QLegendMarker::LegendMarkerTypeXY:
    {
        // Toggle visibility of series
        marker->series()->setVisible(!marker->series()->isVisible());

        // Turn legend marker back to visible, since hiding series also hides the marker
        // and we don't want it to happen now.
        marker->setVisible(true);

        // Dim the marker, if series is not visible
        qreal alpha = 1.0;

        if (!marker->series()->isVisible())
        {
            alpha = 0.5;
        }

        QColor color;
        QBrush brush = marker->labelBrush();
        color        = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setLabelBrush(brush);

        brush = marker->brush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setBrush(brush);

        QPen pen = marker->pen();
        color    = pen.color();
        color.setAlphaF(alpha);
        pen.setColor(color);
        marker->setPen(pen);
        break;
    }
    default:
        break;
    }
}

void ValidationScatterView::addReferenceOriginLine(double x, double y, Qt::PenStyle style, QColor color)
{
    auto* series = new QLineSeries();

    QPen pen(style);
    pen.setColor(color);

    series->append(0, 0);
    series->append(x, y);
    series->setOpacity(0.75);
    series->setPen(pen);

    _chart->addSeries(series);
    series->attachAxis(_axisX);
    series->attachAxis(_axisY);
}

}

