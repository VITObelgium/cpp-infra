#pragma once

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class QTableView;

namespace opaq
{

class ValidationResultsModel;

namespace config
{
struct Component;
}

class Station;

class ValidationLineView : public QWidget
{
    Q_OBJECT

public:
    ValidationLineView(QWidget* parent = 0);

    void setModel(ValidationResultsModel& model);

private:
    void handleMarkerClicked();

    QtCharts::QValueAxis* _axisX;
    QtCharts::QValueAxis* _axisY;
    QtCharts::QChart* _chart;
};
}