#include "infra/exception.h"
#include "infra/log.h"
#include "infra/xmldocument.h"

#include "polytrend.hpp"

namespace rio {

using namespace inf;

// helper routine
static void str2vec(std::vector<double>& x, std::string s)
{
    x.clear();
    std::vector<std::string> s_vec;
    boost::split(s_vec, s, boost::is_any_of(",; \t"), boost::token_compress_on);
    std::transform(s_vec.begin(), s_vec.end(), std::back_inserter(x), boost::lexical_cast<double, std::string>);
}

polytrend::polytrend(const std::string& config_file)
: trendmodel()
{
    _name = "polytrend";

    // Get filename with parameters and import as a boost property tree
    Log::info("Importing trend model config from: {}", config_file);

    auto doc = XmlDocument::load_from_file(config_file);
    if (!doc) {
        throw RuntimeError("unable to load/parse xml file {}", config_file);
    }

    auto rootEl = doc.root_node();
    if (!rootEl || rootEl.name() != "trendmodel") {
        throw RuntimeError("no <trendmodel> root element found in {}");
    }

    // Look up the class... is this the correct config file ?
    auto model_class = rootEl.child("class").trimmed_value();
    if (model_class != "polytrend") {
        throw RuntimeError("trend model config type does not match, expected 'polytrend' got '{}'", model_class);
    }

    // Look up the parameters into boost property tree
    auto trendEl = rootEl.child("trend");
    if (!trendEl) {
        throw RuntimeError("no <trend> element found in {}", config_file);
    }

    std::stringstream ss;
    trendEl.print(ss);

    pt::read_xml(ss, _trend, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments);

    // Look up the station statistics
    auto stEl = rootEl.child("stat_param");
    if (!stEl) {
        throw RuntimeError("no <stat_param> element found in {}", config_file);
    }

    for (auto& e : stEl.children("station")) {
        auto stName = std::string(e.attribute("name"));
        e.print(ss);
        // construct the ptree directly in the map.. nice !
        pt::read_xml(ss, _stat_param[stName], pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments);
    }

    // debug:
    /*
  pt::write_xml(ss, _trend);
  std::cout << ss.str() << std::endl;

  for ( auto& kv: _stat_param )
    std::cout << "Station " << kv.first << std::endl
	      << "  - week h01 avg = " << kv.second.get<double>("station.week.h01.avg") << std::endl
      	      << "  - weekend h24 std = " << kv.second.get<double>("station.weekend.h24.std") << std::endl;
  
  */

    // now get reference levels directly as doubles
    auto cnfEl = rootEl.child("config");
    if (!cnfEl) {
        throw RuntimeError("no <config> element found in {}", config_file);
    }

    cnfEl.print(ss);
    pt::ptree t;
    try {
        pt::read_xml(ss, t, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments);

        _ref_level_avg = t.get<double>("config.avg_trend.ref_level");
        _xlo_avg       = t.get<double>("config.avg_trend.x_lo");
        _xhi_avg       = t.get<double>("config.avg_trend.x_hi");
        _ref_level_std = t.get<double>("config.std_trend.ref_level");
        _xlo_std       = t.get<double>("config.std_trend.x_lo");
        _xhi_std       = t.get<double>("config.std_trend.x_hi");

    } catch (...) {
        RuntimeError("unable to read config from trend xml file");
    }

    std::cout << " avg trend ref_level= " << _ref_level_avg
              << ", x_lo= " << _xlo_avg
              << ", x_hi= " << _xhi_avg << std::endl;
    std::cout << " std trend ref_level= " << _ref_level_std
              << ", x_lo= " << _xlo_std
              << ", x_hi= " << _xhi_std << std::endl;
}

polytrend::~polytrend()
{
}

void polytrend::select(const std::string& aggr, boost::posix_time::ptime tstart)
{
    // Get week/weekend from tstart day
    std::string wk;
    if (tstart.date().day_of_week() == boost::date_time::Saturday ||
        tstart.date().day_of_week() == boost::date_time::Sunday) {
        wk = "weekend.";
    } else {
        wk = "week.";
    }

    // construct aggregation selection
    std::stringstream ss;
    if (!aggr.compare("1h")) {
        // in case of 1h aggregation, add the hour of the day, otherwise just use the aggregation
        long hr = tstart.time_of_day().hours() + 1;
        ss << boost::format("h%02d") % hr;
    } else
        ss << aggr;

    // is re-selecting needed ? --> multiple days which are week/weekend
    std::string selection = wk + ss.str();
    if (selection.compare(_curr_sel)) {
        try {
            str2vec(_p_avg, _trend.get<std::string>("trend." + selection + ".avg"));
            str2vec(_p_avg_err, _trend.get<std::string>("trend." + selection + ".avg_err"));
            str2vec(_p_std, _trend.get<std::string>("trend." + selection + ".std"));
            str2vec(_p_std_err, _trend.get<std::string>("trend." + selection + ".std_err"));
        } catch (std::exception& e) {
            throw RuntimeError(std::string("cannot retrieve trend parameters : ") + e.what());
        }

        _curr_sel = selection;
    } else {
        // parameters remain the same
    }

    /*
  std::cout << "p_avg = ";
  for ( const auto& x : _p_avg ) std::cout << x << " ";
  std::cout << std::endl;
  */
}

void polytrend::detrend(double& v, const std::string& station, const std::vector<double> proxy)
{
    // the beta, or prxy parameter, we only expect 1 here
    double x = proxy[0];

    // get trend scale
    double scale = poly(_p_std, x, _xlo_std, _xhi_std);
    if (scale != 0.) {
        scale = _ref_level_std / scale;
    } else {
        scale = 1.;
    }

    // get trend shift
    double shift = _ref_level_avg - poly(_p_avg, x, _xlo_avg, _xhi_avg);

    // get station average
    double stat_avg = _stat_param[station].get<double>("station." + _curr_sel + ".avg");

    // detrend
    v = std::max(0., (v - stat_avg) * scale + stat_avg + shift);
}

void polytrend::addtrend(double& v, double& e, const std::vector<double> proxy)
{
    double x = proxy[0];

    // get trend scale
    double scale = poly(_p_std, x, _xlo_std, _xhi_std);
    if (scale != 0.) {
        scale = _ref_level_std / scale;
    } else {
        scale = 1.;
    }

    // 1. compute uncertainty as we need the uncorrected v
    double avg_err = poly(_p_avg_err, x);
    double std_err = poly(_p_std_err, x);

    e = sqrt(e * e / scale / scale + avg_err * avg_err +
             pow(std_err * (v - _ref_level_avg) / _ref_level_std, 2.));

    // get trend value
    double trend = poly(_p_avg, x, _xlo_avg, _xhi_avg);

    // 2. compute the re-trended value
    v = v - (_ref_level_avg - trend);
    v = (v - trend) / scale + trend;
}

double polytrend::poly(const std::vector<double>& p, double x)
{
    double y = p[0];
    for (int i = 1; i < p.size(); i++)
        y = y * x + p[i];
    return y;
}

double polytrend::poly(const std::vector<double>& p, double x, double x_lo, double x_hi)
{
    if (x_hi <= x_lo) throw RuntimeError("x_lo,x_hi boundaries not correct");

    switch (p.size()) {
    case 0:
    case 1:
        throw RuntimeError("Invalid polynomial degree (<1)");
        break;

    case 2: // first degree poly

        if (x < x_lo) return poly(p, x_lo);
        if (x > x_hi) return poly(p, x_hi);
        // otherwise default at the end of this function
        break;

    case 3: // second degree poly

        // calculate a and b in a*(x-b)^2+c
        double a = p[0];
        double b = -p[1] / (2. * a);

        // and get the values at x_lo and x_hi
        double y_lo = poly(p, x_lo);
        double y_hi = poly(p, x_hi);

        // the final x_flat values
        double x_flat_lo = 0.;
        double x_flat_hi = 0.;

        if (a < 0) {
            //  here we have a parabola with a maxium ( negative a -> y=-x^2)
            if (y_lo > y_hi) { // we are on the falling side
                x_flat_lo = std::max(b, x_lo);
                x_flat_hi = x_hi;
            } else { // we are on the rising side
                x_flat_lo = x_lo;
                x_flat_hi = std::min(b, x_hi);
            }
        } else {
            // here we have a parabola with a minimum (positive a -> y=x^2)
            if (y_lo > y_hi) { // we are on the falling side
                x_flat_lo = x_lo;
                x_flat_hi = std::min(b, x_hi);
            } else { // we are on the rising side
                x_flat_lo = std::max(b, x_lo);
                x_flat_hi = x_hi;
            }
        }

        if (x < x_flat_lo) return poly(p, x_flat_lo);
        if (x > x_flat_hi) return poly(p, x_flat_hi);
        // otherwise default at the end of this function
        break;
    }

    // default
    return poly(p, x);
}
}
