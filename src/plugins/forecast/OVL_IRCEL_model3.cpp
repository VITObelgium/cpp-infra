#include <fstream>

#include "OVL_IRCEL_model3.h"
#include <opaq.h>

#define epsilon 1e-6

namespace OPAQ
{

using namespace chrono_literals;
using namespace std::chrono_literals;

OVL_IRCEL_model3::OVL_IRCEL_model3()
: p_t2m("P01") // t2m in IRCEL meteo provider
, p_wsp10m("P03") // wind speed 10 m in IRCEL meteo provider
, p_wdir10m("P04") // wind direction 10 m
, p_blh("P07") // boundary layer height
, p_cc("P13") // low cloud cover
, p_rh("P24") // relative humidity
, p_S("P21") // buyltinck-malet S parameter
, p_Transp("P16") // horizontal transport in BL (BLH x mean wind in BLH)
, mor_agg(-1) // default is all
{
    sample_size = 11;
}

/* ============================================================================
     Implementation of the configure method
     ========================================================================== */
void OVL_IRCEL_model3::configure(TiXmlElement* cnf, const std::string& componentName, IEngine&)
{
    setName(componentName);
    try
    {
        // read the path to the architecture files
        this->pattern = XmlTools::getText(cnf, "ffnetfile_pattern");
    }
    catch (ElementNotFoundException& e)
    {
        throw BadConfigurationException(e.what());
    }

    // read morning aggregation (optional)
    try
    {
        this->mor_agg = atoi(XmlTools::getText(cnf, "mor_agg").c_str());
    }
    catch (const ElementNotFoundException&)
    {
        this->mor_agg = 24; // take all available observations for the day
    }

    // read missing value for this model (optional)
    try
    {
        this->missing_value = atoi(XmlTools::getText(cnf, "missing_value").c_str());
    }
    catch (...)
    {
    };
}

/* ============================================================================
     Construct sample for the OVL_IRCEL_model3 configuration
     ========================================================================== */
int OVL_IRCEL_model3::makeSample(double* sample, const Station& st,
                                 const Pollutant& pol, Aggregation::Type aggr,
                                 const chrono::date_time& baseTime, const chrono::date_time& fcTime,
                                 chrono::days fc_hor)
{

    int have_sample = 0; // return code, 0 for success

    // -----------------------
    // Getting data providers --> stored in main model...
    // -----------------------
    DataProvider* obs    = getInputProvider();
    MeteoProvider* meteo = getMeteoProvider();

    // -----------------------
    // Get the meteo input
    // -----------------------
    // BLH for dayN, offsets relative from fcTime (set by setBaseTime in the meteo provider)
    auto t1 = fcTime;
    auto t2 = fcTime + 1_d - meteo->getTimeResolution();

    TimeSeries<double> blh = meteo->getValues(t1, t2, st.getMeteoId(), p_blh);
    TimeSeries<double> cc  = meteo->getValues(t1, t2, st.getMeteoId(), p_cc);
    TimeSeries<double> t2m = meteo->getValues(t1, t2, st.getMeteoId(), p_t2m);
    TimeSeries<double> rh  = meteo->getValues(t1, t2, st.getMeteoId(), p_rh);
    TimeSeries<double> S   = meteo->getValues(t1, t2, st.getMeteoId(), p_S);
    TimeSeries<double> tra = meteo->getValues(t1, t2, st.getMeteoId(), p_Transp);

    t1 = fcTime - 12h;                              // dayN-1 : 12:00 to end, including 12:00
    t2 = fcTime + 12h - meteo->getTimeResolution(); // day N : 00:00 to 12:00 (excluding)

    TimeSeries<double> wsp  = meteo->getValues(t1, t2, st.getMeteoId(), p_wsp10m);
    TimeSeries<double> wdir = meteo->getValues(t1, t2, st.getMeteoId(), p_wdir10m);

    // -----------------------
    // build sample
    // -----------------------

    // 0. -------------------------------------------------------------------------------
    // sample[0] is the mean morning concentration of the measured pollutant we're trying to forecast
    t1 = std::chrono::floor<chrono::days>(baseTime);
    t2 = t1 + std::chrono::hours(mor_agg - 1); // mor_agg uur of eentje aftrekken ?

    auto xx_morn = obs->getValues(t1, t2, st.getName(), pol.getName()); // no aggregation
    double xx    = mean_missing(xx_morn.values(), obs->getNoData());
    if (fabs(xx - obs->getNoData()) > epsilon)
        sample[0] = log(1 + xx);
    else
        have_sample++;

    // 1. -------------------------------------------------------------------------------
    // sample[1] is the mean concentration of the measured pollutant of day-1
    t1 = std::chrono::floor<chrono::days>(baseTime) - 1_d;
    t2 = std::chrono::floor<chrono::days>(baseTime) - obs->getTimeResolution(); // 1x meteto TimeResultion aftrekken : - getTimeResolution();

    auto xx_yest = obs->getValues(t1, t2, st.getName(), pol.getName());
    xx           = mean_missing(xx_yest.values(), obs->getNoData());
    if (fabs(xx - obs->getNoData()) > epsilon)
        sample[1] = log(1 + xx);
    else
        have_sample++;

    // 2. -------------------------------------------------------------------------------
    // sample[2] is the mean/min/max boundary layer height of the forecast day, depending on
    // the aggregation time & polluant.
    if (aggr == Aggregation::Max1h) {
        if (!pol.getName().compare("o3")) {
            // take the maximum BLH for O3 forecasts
            xx = max_missing(blh.values(), meteo->getNoData(p_blh));
        }
        else
        {
            // take the minimum BLH
            xx = min_missing(blh.values(), meteo->getNoData(p_blh));
        }
    }
    else
    {
        // also for max8h we use the daily averages
        xx = mean_missing(blh.values(), meteo->getNoData(p_blh));
    }
    if (fabs(xx - meteo->getNoData(p_blh)) > epsilon)
        sample[2] = log(1 + xx);
    else
    {
        have_sample++;
        // TODO perhaps provide some kind of climatology to fill the missing sample...
    }

    // 3. -------------------------------------------------------------------------------
    // sample[3] is the mean medium cloud cover for dayN, no logtransform
    xx = mean_missing(cc.values(), meteo->getNoData(p_cc));
    if (fabs(xx - meteo->getNoData(p_cc)) > epsilon)
        sample[3] = xx;
    else
        have_sample++;

    // 4. -------------------------------------------------------------------------------
    // sample[4] is the 2m temperature of dayN
    if (aggr == Aggregation::Max1h) {
        if (!pol.getName().compare("o3")) {
            xx = max_missing(t2m.values(), meteo->getNoData(p_t2m));
        }
        else
        {
            xx = min_missing(t2m.values(), meteo->getNoData(p_t2m));
        }
    }
    else
    {
        xx = mean_missing(t2m.values(), meteo->getNoData(p_t2m));
    }
    if (fabs(xx - meteo->getNoData(p_t2m)) > epsilon)
        sample[4] = xx; // no logtransform
    else
        have_sample++;

    // 5. -------------------------------------------------------------------------------
    // sample[5] is the relative humidity
    if (aggr == Aggregation::Max1h) {
        if (!pol.getName().compare("o3")) {
            xx = min_missing(rh.values(), meteo->getNoData(p_rh)); // take the min here !!!
        }
        else
        {
            xx = max_missing(rh.values(), meteo->getNoData(p_rh));
        }
    }
    else
    {
        xx = mean_missing(rh.values(), meteo->getNoData(p_rh));
    }
    if (fabs(xx - meteo->getNoData(p_rh)) > epsilon)
        sample[5] = xx; // no logtransform
    else
        have_sample++;

    // 6. -------------------------------------------------------------------------------
    // sample[6] is the buyltinck-malet parameter
    if (aggr == Aggregation::Max1h) {
        if (!pol.getName().compare("o3")) {
            xx = min_missing(S.values(), meteo->getNoData(p_S)); // take the min here !!!
        }
        else
        {
            xx = max_missing(S.values(), meteo->getNoData(p_S));
        }
    }
    else
    {
        xx = mean_missing(S.values(), meteo->getNoData(p_S));
    }
    if (fabs(xx - meteo->getNoData(p_S)) > epsilon)
        sample[6] = xx; // no logtransform
    else
        have_sample++;

    // 7. -------------------------------------------------------------------------------
    // sample[7] is the buyltinck-malet parameter
    if (aggr == Aggregation::Max1h) {
        if (!pol.getName().compare("o3")) {
            xx = max_missing(tra.values(), meteo->getNoData(p_Transp));
        }
        else
        {
            xx = min_missing(tra.values(), meteo->getNoData(p_Transp));
        }
    }
    else
    {
        xx = mean_missing(tra.values(), meteo->getNoData(p_Transp));
    }
    if (fabs(xx - meteo->getNoData(p_Transp)) > epsilon)
        sample[7] = log(1 + xx); // no logtransform
    else
        have_sample++;

    // 8. and 9. -------------------------------------------------------------------------
    // sample[8] and sample[9] are zoneal & meridional wind vectors, note that above
    // we have used the nodata value of the wsp10m
    double x_vec, y_vec;
    bool ok;
    Math::winddir(&x_vec, &y_vec, wdir.values(), meteo->getNoData(p_wdir10m), &ok);
    if (!ok) {
        have_sample++;
    }
    else
    {
        xx = mean_missing(wsp.values(), meteo->getNoData(p_wsp10m)); // average wind speed
        if (fabs(xx - meteo->getNoData(p_wsp10m)) < epsilon)
            have_sample++;
        else
        {
            sample[8] = x_vec * xx;
            sample[9] = y_vec * xx;
        }
    }

    // 10. ------------------------------------------------------------------------------
    // get the weekday : week/weekend ?
    sample[10] = chrono::is_weekend(fcTime) ? 1 : 0;

    return have_sample;
}
}

OPAQ_REGISTER_PLUGIN(OPAQ::OVL_IRCEL_model3);
