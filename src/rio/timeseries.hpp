#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>

namespace rio {

class timeseries
{
public:
    timeseries();
    ~timeseries();

    size_t size() const
    {
        return _times.size();
    }
    const std::vector<double>& values(void) const
    {
        return _values;
    }
    const boost::posix_time::time_duration& interval()
    {
        return _period;
    }

    void insert(const boost::posix_time::time_period& dt, double value, bool overwrite = false);

    boost::posix_time::ptime first_time() const
    {
        return _times.front().begin();
    }

    boost::posix_time::ptime last_time() const
    {
        return _times.back().last();
    } // end() is the first after the last

    // Internally RIO works with the start times of the interval for looking up values etc...
    // Nevertheless, in the datahandler & timeseries objects, the full timeintervals are stored...
    double get(boost::posix_time::ptime tstart) const;
    double get(boost::posix_time::ptime tstart, bool& missing) const;

public:
    friend std::ostream& operator<<(std::ostream& out, const timeseries& ts);

private:
    std::vector<double> _values;
    std::vector<boost::posix_time::time_period> _times;
    boost::posix_time::time_duration _period; // store the time interval for the series

    double _missing_value;
};

}
