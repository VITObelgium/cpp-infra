#pragma once

#include <QtWidgets/QWidget>

class QTableView;

namespace QtCharts
{
    class QScatterSeries;
    class QValueAxis;
    class QChart;
}

namespace opaq
{

class ValidationResultsModel;

namespace config
{
struct Component;
}

class Station;

class ValidationScatterView : public QWidget
{
    Q_OBJECT

public:
    ValidationScatterView(QWidget* parent = 0);

    void setModel(ValidationResultsModel& model);

private:
    void handleMarkerHovered(bool status);
    void handleMarkerClicked();
    void addReferenceOriginLine(double x, double y, Qt::PenStyle style, QColor color);

    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;

    ValidationResultsModel* _model;
    std::vector<QtCharts::QScatterSeries*> _modelSeries;
};
}