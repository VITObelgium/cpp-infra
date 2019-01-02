#include "infra/exception.h"
#include "infra/log.h"
#include "infra/xmldocument.h"

#include "strfun.hpp"

#include "mlrtrend.hpp"

namespace rio {

using namespace inf;

mlrtrend::mlrtrend(const std::string& config_file)
: trendmodel()
{
    _name = "mlrtrend";

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
    if (model_class != "mlrtrend") {
        throw RuntimeError("trend model config type does not match, expected 'mlrtrend' got '{}'", model_class);
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

        // needed ?
        _ref_level_avg = t.get<double>("config.avg_trend.ref_level");

        rio::strfun::str2vec(_xlo_avg, t.get<std::string>("config.avg_trend.x_lo"));
        rio::strfun::str2vec(_xhi_avg, t.get<std::string>("config.avg_trend.x_hi"));

        // needed ?
        _ref_level_std = t.get<double>("config.std_trend.ref_level");

        rio::strfun::str2vec(_xlo_std, t.get<std::string>("config.std_trend.x_lo"));
        rio::strfun::str2vec(_xhi_std, t.get<std::string>("config.std_trend.x_hi"));

    } catch (...) {
        RuntimeError("unable to read config from trend xml file");
    }

    std::cout << " avg trend ref_level= " << _ref_level_avg
              << ", x_lo= [";
    for (auto& v : _xlo_avg)
        std::cout << " " << v;
    std::cout << " ], x_hi= [";
    for (auto& v : _xhi_avg)
        std::cout << " " << v;
    std::cout << " ]" << std::endl;

    std::cout << " std trend ref_level= " << _ref_level_std
              << ", x_lo= [";
    for (auto& v : _xlo_std)
        std::cout << " " << v;
    std::cout << " ], x_hi= [";
    for (auto& v : _xhi_std)
        std::cout << " " << v;
    std::cout << " ]" << std::endl;
}

mlrtrend::~mlrtrend()
{
}

void mlrtrend::select(const std::string& aggr, boost::posix_time::ptime tstart)
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
        auto hr = tstart.time_of_day().hours() + 1;
        ss << boost::format("h%02d") % hr;
    } else
        ss << aggr;

    // is re-selecting needed ? --> multiple days which are week/weekend
    std::string selection = wk + ss.str();
    if (selection.compare(_curr_sel)) {
        try {
            rio::strfun::str2vec(_p_avg, _trend.get<std::string>("trend." + selection + ".avg"));
            rio::strfun::str2vec(_p_std, _trend.get<std::string>("trend." + selection + ".std"));

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

void mlrtrend::detrend(double& v, const std::string& station, const std::vector<double> proxy)
{
    // get trend scale
    double scale = mlr_apply(_p_std, proxy, _xlo_std, _xhi_std);
    if (scale != 0.) {
        scale = _ref_level_std / scale;
    } else {
        scale = 1.;
    }

    // get trend shift
    double shift = _ref_level_avg - mlr_apply(_p_avg, proxy, _xlo_avg, _xhi_avg);

    // get station average : should this not be _p_avg[0] in the trend parametrs for mlrtrend ??
    double stat_avg = _stat_param[station].get<double>("station." + _curr_sel + ".avg");

    // detrend
    v = std::max(0., (v - stat_avg) * scale + stat_avg + shift);
}

void mlrtrend::addtrend(double& v, double& e, const std::vector<double> proxy)
{
    // get trend scale
    double scale = mlr_apply(_p_std, proxy, _xlo_std, _xhi_std);
    if (scale != 0.) {
        scale = _ref_level_std / scale;
    } else {
        scale = 1.;
    }

    double trend = mlr_apply(_p_avg, proxy, _xlo_avg, _xhi_avg);

    // 2. compute the re-trended value
    v = v - (_ref_level_avg - trend);
    v = (v - trend) / scale + trend;

    // 3. set error to 0 here...
    e = 0.;
}

double mlrtrend::mlr_apply(const std::vector<double>& c, const std::vector<double>& x, const std::vector<double>& xlo, const std::vector<double>& xhi)
{
    if (!((c.size() == x.size()) && (x.size() == xlo.size()) && (x.size() == xhi.size())))
        throw RuntimeError("Input proxy/coefficient/xlo/xhi vector sizes dont match in mlrtrend::apply...");

    double y = 0.;
    for (unsigned int i = 0; i < c.size(); i++) {
        if (x[i] < xlo[i]) {
            y += c[i] * xlo[i];
        } else if (x[i] > xhi[i]) {
            y += c[i] * xhi[i];
        } else {
            y += c[i] * x[i];
        }
    }
    return y;
}

}
