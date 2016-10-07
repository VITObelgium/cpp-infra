#pragma once

#include "TimeInterval.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class QTableView;

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
    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;
};
}