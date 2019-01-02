#include <iostream>
#include <stdexcept>
#include <string>

#include "timeseries.hpp"

namespace rio {

timeseries::timeseries()
{
    _missing_value = -9999;
}

timeseries::~timeseries()
{
}

double timeseries::get(boost::posix_time::ptime tstart) const
{
    bool missing;
    return this->get(tstart, missing);
}

double timeseries::get(boost::posix_time::ptime tstart, bool& missing) const
{
    boost::posix_time::time_duration offset = tstart - _times.front().begin();
    if (offset.ticks() % _period.ticks())
        throw std::runtime_error("requested time coordinate does not match in timeseries::get()...");

    long idx = static_cast<long>(offset.ticks() / _period.ticks());
    if (idx < 0 || idx >= this->size()) {
        missing = true;
        return _missing_value;
    }

    // the time can be valid, but the data is missing in the datafile...
    if (fabs(_values[idx] - _missing_value) < 1e-8)
        missing = true;
    else
        missing = false;

    return _values[idx];
}

void timeseries::insert(const boost::posix_time::time_period& dt, double value, bool overwrite)
{
    // insert first elements
    if (this->size() == 0) {
        _period = dt.length();
        _times.push_back(dt);
        _values.push_back(value);
        return;
    }

    // check period
    if (_period != dt.length())
        throw std::runtime_error("timeseries insert period does not match...");

    // do we insert one past the last (ususally so efficient to have it here)
    if (_times.back().end() == dt.begin()) {
        _times.push_back(dt);
        _values.push_back(value);
        return;
    }

    // check for phase
    boost::posix_time::time_duration offset = dt.begin() - _times.front().begin();
    long noff                               = static_cast<long>(offset.ticks() / _period.ticks());
    if (offset.ticks() % _period.ticks())
        throw std::runtime_error("timeseries insert out of phase...");

    // insert depending on position
    if (dt.begin() > _times.back().end()) {
        // at the end with a gap
        offset = dt.begin() - _times.back().end();
        long n = static_cast<long>(offset.ticks() / _period.ticks());
        boost::posix_time::time_period ddt(_times.back().end(), _period);
        for (int i = 0; i < n; i++) {
            _times.push_back(ddt);
            _values.push_back(_missing_value);
            ddt.shift(_period);
        }
        _times.push_back(dt);
        _values.push_back(value);

    } else if (dt.end() < _times.front().begin()) {
        // before beginning with a gap
        offset = _times.front().begin() - dt.end();
        long n = static_cast<long>(offset.ticks() / _period.ticks());
        boost::posix_time::time_period ddt(_times.front().begin(), _period);
        for (int i = 0; i < n; i++) {
            ddt.shift(-_period); // first shift here
            _times.insert(_times.begin(), ddt);
            _values.insert(_values.begin(), _missing_value);
        }
        _times.insert(_times.begin(), dt);
        _values.insert(_values.begin(), value);

    } else {
        // we are past the first element ( or on it), so noff is positive...
        if (overwrite) {
            if ((noff < 0) || (noff >= this->size())) throw std::runtime_error("error in offset calculation...");
            _values[noff] = value;
        }
    }

    return;
}

std::ostream& operator<<(std::ostream& out, const timeseries& ts)
{
    for (size_t i = 0; i < ts.size(); i++)
        out << ts._times[i] << " " << ts._values[i] << std::endl;
    return out;
}

}
