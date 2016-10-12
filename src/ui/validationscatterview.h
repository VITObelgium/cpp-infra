#pragma once

#include "TimeInterval.h"

#include <QtWidgets/QWidget>

class QTableView;

namespace QtCharts
{
    class QScatterSeries;
    class QValueAxis;
    class QChart;
}

namespace OPAQ
{

class ValidationResultsModel;

namespace Config
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
    void handleMarkerClicked();
    void addReferenceOriginLine(double x, double y, Qt::PenStyle style, QColor color);

    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;

    std::vector<QtCharts::QScatterSeries*> _modelSeries;
};
}