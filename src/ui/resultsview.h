#pragma once

#include "stationresultsmodel.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class QTableView;

namespace OPAQ
{

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
    void setModels(const std::vector<Config::Component*>& model);
    void updateResultsForStation(ForecastBuffer& buffer,
                                 DateTime baseTime,
                                 const std::string& station,
                                 TimeInterval forecastHorizon,
                                 const std::string& pollutantId,
                                 Aggregation::Type agg);

private:
    StationResultsModel _model;
    QTableView* _tableView;
    int _rows;
    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;
};
}