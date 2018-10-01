#pragma once

#include "data/MeteoProvider.h"

#include <map>

namespace opaq {

class IRCELMeteoProvider : public MeteoProvider
{
public:
    IRCELMeteoProvider();

    static std::string name();

    // OPAQ::Component methods
    // throws BadConfigurationException
    void configure(const inf::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    // OPAQ::MeteoProvider methods

    /**
   * Ignored
   */
    virtual std::chrono::hours getTimeResolution() override;

    /**
   * throws an exception: use getNoData(const std::string &) instead
   */
    virtual double getNoData(const std::string& parameterId) override;

    /**
   * Return the values between t1 and t2 including the boundaries !
   */
    virtual TimeSeries<double> getValues(const chrono::date_time& t1,
        const chrono::date_time& t2,
        const std::string& meteoId,
        const std::string& paramId) override;

private:
    void throwIfNotConfigured();
    void readFile(const std::string& meteoId, const std::string& parameterId);

    // -- the dynamic meteo data buffer
    std::map<std::string, std::map<std::string, TimeSeries<double>>> _buffer;

    // -- the buffer with nodata values
    std::map<std::string, double> _nodata;

    // -- file pattern for reading the ECMWF txt files generated by IRCEL
    std::string _pattern;

    bool _configured;                   //! is the object completely configured
    std::chrono::hours _timeResolution; //! what is the time resolution of the meteo data ?
    size_t _nsteps;                     //! number of steps in a line in the datafile
    chrono::date_time _bufferStartDate; //! requested buffer start date, if not given, then all file is stored
    bool _bufferStartReq;               //! is this requested ?
    int _backsearch;                    //! how many days are we looking in the past to find data file
};
}
