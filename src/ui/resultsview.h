#pragma once

#include "TimeInterval.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class QTableView;

namespace OPAQ
{

class StationResultsModel;

namespace Config
{

struct Component;
}

class Station;

class ResultsView : public QWidget
{
    Q_OBJECT

public:
    ResultsView(QWidget* parent = 0);

    void setForecastHorizon(TimeInterval forecastHorizon);
    void setModels(StationResultsModel& model, const std::vector<Config::Component>& modelComponents);

private:
    int _rows;
    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;
};
}