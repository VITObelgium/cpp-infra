#pragma once

#include "../Component.h"
#include "../DateTime.h"
#include "../TimeSeries.h"

namespace opaq
{

class MeteoProvider : public Component
{
public:
    /**
	 * Get the time resolution of the provided data in the meteo data provider
	 */
    virtual std::chrono::hours getTimeResolution() = 0;

    /**
	 * Get the nodata placeholder for the given parameter
	 * For example: for meteo data, the no data placeholder might be different for each
	 * parameter.
	 */
    virtual double getNoData(const std::string& id) = 0;

    /**
	 * Return the values between t1 and t2 including the boundaries!
	 */
    virtual opaq::TimeSeries<double> getValues(const chrono::date_time& t1,
                                               const chrono::date_time& t2,
                                               const std::string& meteoId,
                                               const std::string& paramId) = 0;

    /**
	 * Set basetime for the meteo provider
	 */
    void setBaseTime(const chrono::date_time& t) { _baseTime = t; }
    chrono::date_time getBaseTime() { return _baseTime; }

protected:
    chrono::date_time _baseTime;
};

}
