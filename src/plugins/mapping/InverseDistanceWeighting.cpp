
#include "InverseDistanceWeighting.h"
#include "PluginRegistration.h"
#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "data/IGridProvider.h"
#include "data/IStationInfoProvider.h"
#include "data/IMappingBuffer.h"
#include "data/DataProvider.h"
#include "tools/XmlTools.h"

#include <tinyxml.h>

namespace opaq
{

using namespace chrono_literals;
using namespace std::chrono_literals;

InverseDistanceWeighting::InverseDistanceWeighting()
: Model("IDWModel")
, _powerParam(0.0)
{
}

std::string InverseDistanceWeighting::name()
{
    return "idwmodel";
}

void InverseDistanceWeighting::configure(TiXmlElement* cnf, const std::string& componentName, IEngine&)
{
    setName(componentName);

    _powerParam = XmlTools::getChildValue<double>(cnf, "power_parameter", 1.0);
    _gisType = XmlTools::getChildValue<std::string>(cnf, "gis_type", "clc06d");
}

static double calculateDistance(double x1, double y1, double x2, double y2)
{
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

void InverseDistanceWeighting::run()
{
    auto basetime = getBaseTime();
    auto& dataProvider = getInputProvider();
    auto& grid = getGridProvider().getGrid(getPollutant().getName(), GridType::Grid4x4);
    auto stations = getAQNetworkProvider().getAQNetwork().getStations();

    std::vector<double> results;
    results.reserve(stations.size());

    for (auto& cell : grid)
    {
        const auto x = cell.getXc();
        const auto y = cell.getYc();

        double num = 0.0;
        double den = 0.0;
        double idw = 0.0;
        bool onStation = false;

        for (auto& station : stations)
        {
            auto values = dataProvider.getValues(basetime, basetime + 23h, station.getName(), getPollutant().getName(), Aggregation::Max1h);
            if (values.isEmpty())
            {
                continue;
            }
            
            auto distance = calculateDistance(x, y, station.getX(), station.getY());
            if (distance == 0.0)
            {
                idw = values.valueAt(basetime);
                onStation = true;
                continue;
            }
            else
            {
                const auto value = values.valueAt(basetime);
                if (std::abs(value - dataProvider.getNoData()) <= std::numeric_limits<double>::epsilon())
                {
                    continue; // nodata
                }

                const auto wix = 1.0 / std::pow(distance, _powerParam);
                num += wix * values.valueAt(basetime);
                den += wix;
            }
        }

        if (!onStation)
        {
            idw = num / den;
        }

        results.push_back(idw);

        _logger->info("Cell: {} idw: {}", cell.getId(), idw);
    }

    getMappingBuffer().addResults(0, results);
}

OPAQ_REGISTER_STATIC_PLUGIN(InverseDistanceWeighting)

}

