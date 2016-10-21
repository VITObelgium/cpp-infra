/*
 *
 * AsciiForecastWriter.h
 *
 */
#ifndef AsciiForecastWriter_H_
#define AsciiForecastWriter_H_

#include <string>
#include <vector>

#include "opaq.h"

namespace OPAQ
{

class AsciiForecastWriter : public ForecastOutputWriter
{

public:
    AsciiForecastWriter();
    virtual ~AsciiForecastWriter();

    static const std::string BASETIME_PLACEHOLDER;
    static const std::string POLLUTANT_PLACEHOLDER;
    static const std::string AGGREGATION_PLACEHOLDER;

    // OPAQ::Component methods
    // throws (BadConfigurationException)
    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    // OPAQ::ForecastOutputWriter methods
    void write(Pollutant* pol, Aggregation::Type aggr, const chrono::date_time& baseTime) override;

private:
    Logger _logger;
    std::string _filename;
    std::vector<std::string> _models; //! a list of models to output
    std::string _title;
    std::string _header;
    bool _enable_fields;
    char _sepchar;     //! fields separation character, default is tab
    bool _fctime_full; //! output full fctime
    bool _full_output; //! output all the stations, if false will only ouptut stations that measure the pollutant
};

} // namespace

#endif /* #ifndef AsciiForecastWriter_H_ */
