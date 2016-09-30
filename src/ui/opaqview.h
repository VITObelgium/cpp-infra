#pragma once

#include "DateTime.h"
#include "Aggregation.h"
#include "TimeInterval.h"

#include <QComboBox>
#include <QWidget>
#include <memory>

namespace Ui
{
class OpaqView;
}

namespace OPAQ
{

namespace Config
{
struct Component;
}

class Station;
class ForecastBuffer;

class OpaqView : public QWidget
{
    Q_OBJECT

public:
    explicit OpaqView(QWidget* parent = nullptr);
    virtual ~OpaqView();

    void setStations(const std::vector<Station*>& stations);
    void setModels(const std::vector<Config::Component*>& models);
    void setBaseTime(const DateTime& baseTime);
    void setForecastHorizon(const TimeInterval& forecastHorizon);
    void setForecastBuffer(ForecastBuffer& buffer);
    void setPollutantId(const std::string& pollutantId);
    void setAggregationType(Aggregation::Type agg);

    void updateResultsForCurrentStation();

private:
    void stationChanged();

    std::unique_ptr<Ui::OpaqView> _ui;
    DateTime _baseTime;
    TimeInterval _forecastHorizon;
    std::string _pollutantId;
    Aggregation::Type _aggregation;
    ForecastBuffer* _buffer;
};
}
