/*
 * RioObsProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef RIOOBSPROVIDER_H_
#define RIOOBSPROVIDER_H_

#include "data/DataProvider.h"

#include <unordered_map>

namespace opaq {

class RioObsProvider : public DataProvider
{
public:
    RioObsProvider();

    static std::string name();

    // OPAQ::Component methods

    /**
   * RioObsProvider configuration:
   *
   * <file_pattern>/full/path/to/data/file/name_of_file_for_pollutant_%POL%_.txt</file_pattern>
   */

    // throws BadConfigurationException
    void configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    // OPAQ::DataProvider methods
    std::chrono::hours getTimeResolution() override;

    double getNoData() override;

    /**
   * Simply return the observations for the station & pollutant Id between the given dates
   * the aggregation type returns what aggregation to give, default (when the 4th argument is
   * omitted, the base resolution is returned)
   *
   * The method relies on the select method of the TimeSeries template class which explicitly
   * fills up the requested series. The timestep is set based upon the aggregation time
   */
    virtual TimeSeries<double> getValues(const chrono::date_time& t1, const chrono::date_time& t2,
        const std::string& stationId, const std::string& pollutantId,
        Aggregation::Type aggr = Aggregation::None) override;

private:
    double _noData;
    std::chrono::hours _timeResolution;
    std::string _pattern;
    bool _configured;

    unsigned int _nvalues; //< number of values on a line

    // order of the map :
    // pollutant -> ( aggregation -> ( station -> data ) )

    // buffer for the aggregations in the RIO files, includes the raw as well
    // note that in the RIO files, these aggregations are pre-calculated, but this
    // does not necessarily have to be the case...
    // station --> pollutant --> aggregation --> data
    std::unordered_map<std::string, std::unordered_map<Aggregation::Type, std::unordered_map<std::string, TimeSeries<double>>>> _buffer; //< the data buffer for the aggregations

    TimeSeries<double>& _getTimeSeries(const std::string& pollutant,
        const std::string& station, Aggregation::Type aggr);

    void readFile(const std::string& pollutant);
};

} /* namespace OPAQ */
#endif /* RIOOBSPROVIDER_H_ */
