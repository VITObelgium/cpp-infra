#pragma once

#include "DateTime.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class QTableView;

namespace opaq
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

    void setForecastHorizon(chrono::days forecastHorizon);
    void setModels(StationResultsModel& model, const std::vector<Config::Component>& modelComponents);

private:
    void handleMarkerClicked();

    int _rows;
    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;
};
}