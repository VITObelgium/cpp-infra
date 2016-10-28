/*
 * TimeSeries.h
 *
 *  Created on: Dec 18, 2015
 *      Author: bino
 */

#pragma once

#include "DateTime.h"
#include "Exceptions.h"
#include <fstream>
#include <algorithm>

namespace OPAQ
{

/**
 * Basic template class for OPAQ timeseries
 */
template <class T>
class TimeSeries
{
public:
    size_t size() const { return _datetimes.size(); }
    void setNoData(const T& missing_value) { _missing_value = missing_value; }
    const T& getNoData() const { return _missing_value; }
    bool isEmpty() const { return _datetimes.empty(); }

    bool isConsistent(const TimeSeries<T>& t) const
    {
        if (t.size() != size()) return false;
        for (unsigned int i = 0; i < size(); i++)
            if (_datetimes[i] != t.datetimes().at(i)) return false;
        return true;
    }

    const std::vector<T>& values(void) const
    {
        return _values;
    }

    const std::vector<chrono::date_time>& datetimes() const
    {
        return _datetimes;
    }

    // ======================
    // insert functionality
    // ======================
    /**
	 * Insert elements into the timeseries, given as a datatime object and a
	 * value (of type T)
     * Throws OutOfBoundsException
	 */
    void insert(const chrono::date_time&, const T&);
    /**
	 * Merges the given timeseries into this timeseries
	 * value (of type T). When duplicate datetimestamps are present,
	 * if overwrite is true, the existing value will be overwritten otherwise
	 * the old values will be kept...
	 */
    void merge(const TimeSeries<T>& dt, bool overwrite = true);

    // ===================
    // element removal
    // ===================
    void clear()
    {
        _values.clear();
        _datetimes.clear();
    }

    void reserve(size_t size)
    {
        _values.reserve(size);
        _datetimes.reserve(size);
    }

    void remove(const chrono::date_time&);
    void removeRange(const chrono::date_time&, const chrono::date_time&, bool with_ends = false);
    void removeBefore(const chrono::date_time&);
    void removeAfter(const chrono::date_time&);

    // ===================
    // search algorithms
    // ===================

    // does the timeseries contain the given datetime
    bool contains(const chrono::date_time& dt) const
    {
        return std::binary_search(_datetimes.begin(), _datetimes.end(), dt);
    }

    // return the first date time in the timeseries
    const chrono::date_time& firstDateTime() const
    {
        return _datetimes.front();
    };
    // return the last date time in the timeseries
    const chrono::date_time& lastDateTime() const
    {
        return _datetimes.back();
    };
    // return the index of the date time, -1 if not found
    int index(const chrono::date_time& dt) const
    {
        for (unsigned int i = 0; i < size(); i++)
            if (_datetimes[i] == dt) return i;
        return -1; // not found
    }
    // return first index after the given datetime
    int indexOfFirstAfter(const chrono::date_time& dt) const
    {
        if (dt == chrono::date_time()) return -1;
        if (dt >= lastDateTime()) return -1;
        if (dt < firstDateTime()) return 0;
        for (unsigned int i = 0; i < size(); i++)
            if (_datetimes[i] > dt) return i;
        return -1; // not found
    }
    // return first index after the given datetime
    int indexOfLastBefore(const chrono::date_time& dt) const
    {
        if (dt == chrono::date_time()) return -1;
        if (dt <= firstDateTime()) return -1;
        if (dt > lastDateTime()) return index(lastDateTime());
        for (unsigned int i = 0; i < size(); i++)
            if (_datetimes[i] >= dt) return i - 1;
        return -1; // not found
    }

    // throws OutOfBoundsException
    chrono::date_time datetime(size_t i) const
    {
        if (i >= size()) throw RunTimeException("index out of range");
        return _datetimes.at(i);
    }

    // throws OutOfBoundsException
    T value(size_t i) const
    {
        if (i >= size()) throw RunTimeException("index out of range");
        return _values.at(i);
    }

    // throws OutOfBoundsException
    T value(const chrono::date_time& dt) const
    {
        if (!contains(dt)) throw RunTimeException("datetime not found in timeseries");
        return _values[index(dt)];
    }
    // this one returns either the value at dt if present, or the last value before the
    // given timestamp (as the at that time most recent value)
    // throws OutOfBoundsException
    T valueAt(const chrono::date_time& dt) const;

    /**
	 * subset the timeseries, returns a new timeseries with the subset of the
	 * object, this version just gives all the values, without wondering whether
	 * there is any gaps
	 */
    TimeSeries<T> select(const chrono::date_time& t1, const chrono::date_time& t2) const;

    /**
	 * Subset the timeseries, returns a new timeseries with the subset of the
	 * object, this version fills up the gaps with the nodata value.
	 */
    TimeSeries<T> select(const chrono::date_time& t1, const chrono::date_time& t2, std::chrono::seconds step) const;

    // write the timeseries to a file
    void write(std::string fname)
    {
        std::ofstream fs;
        fs.open(fname, std::ios::out | std::ios::trunc); // open for output & truncate if exists
        fs << *this;
        fs.close();
        return;
    }

protected:
    std::vector<T> _values;
    std::vector<chrono::date_time> _datetimes;
    T _missing_value;
};

/**
 *  Timeseries insert function.
 *  - If the timeseries is empty or the date is after the last date, push it back...
 *  - If the index is before the first index, simply insert
 *  - If the timeseries already contains the datetime, replace the value...
 *  - Otherwise, lookup the first index after the date to insert
 *  Throws OutOfBoundsException
 */
template <class T>
void TimeSeries<T>::insert(const chrono::date_time& dt, const T& val)
{

    if (dt == chrono::date_time()) throw RunTimeException("invalid datetime given");
    // if the timeseries is empty or the date is after the last date, push it back...
    if (isEmpty() || dt > lastDateTime()) {
        _datetimes.push_back(dt);
        _values.push_back(val);
        return;
    }
    // if the index is before the first index
    if (dt < firstDateTime()) {
        _datetimes.insert(_datetimes.begin(), dt);
        _values.insert(_values.begin(), val);
        return;
    }
    // if the timeseries contains the datetime, replace the value
    if (contains(dt)) {
        _values[index(dt)] = val;
        return;
    }
    // otherwise, lookup the first index after the date to insert
    int i = indexOfFirstAfter(dt);
    if (i >= 0) {
        _datetimes.insert(_datetimes.begin() + i, dt);
        _values.insert(_values.begin() + i, val);
    }

    return;
}

template <class T>
void TimeSeries<T>::merge(const TimeSeries<T>& ts, bool overwrite)
{

    for (unsigned int i = 0; i < ts.size(); i++)
    {
        if (!contains(ts.datetime(i)))
        {
            insert(ts.datetime(i), ts.value(i));
        }
        else if (overwrite)
        {
            insert(ts.datetime(i), ts.value(i));
        }
    }
}

// remove function
template <class T>
void TimeSeries<T>::remove(const chrono::date_time& dt)
{
    if (dt == chrono::date_time()) return;
    int i = index(dt);
    if (i >= 0) {
        _datetimes.erase(_datetimes.begin() + i);
        _values.erase(_values.begin() + i);
    }
    return;
}

template <class T>
void TimeSeries<T>::removeRange(const chrono::date_time& dt1, const chrono::date_time& dt2, bool with_ends)
{
    if (dt1 == chrono::date_time()) return;
    if (dt2 == chrono::date_time()) return;

    // whole range ?
    if ((dt1 < firstDateTime()) && (dt2 > lastDateTime())) {
        clear();
        return;
    }

    int i1 = indexOfFirstAfter(dt1);
    int i2 = indexOfLastBefore(dt2);

    if (i1 < 0) return; // first index in range to be remove is after the timeseries
    if (i2 < 0) return; // last index in range to be removed is before the first element in timeseries

    _values.erase(_values.begin() + i1, _values.begin() + i2 + 1);
    _datetimes.erase(_datetimes.begin() + i1, _datetimes.begin() + i2 + 1);

    if (with_ends) {
        remove(dt1);
        remove(dt2);
    }

    return;
}

template <class T>
void TimeSeries<T>::removeBefore(const chrono::date_time& t)
{
    if (t == chrono::date_time()) return;
    int idx = indexOfLastBefore(t);
    if (idx < 0) {
        _values.erase(_values.begin(), _values.begin() + idx + 1);
        _datetimes.erase(_datetimes.begin(), _datetimes.begin() + idx + 1);
    }
    return;
}

template <class T>
void TimeSeries<T>::removeAfter(const chrono::date_time& t)
{
    if (t == chrono::date_time()) return;
    int idx = indexOfFirstAfter(t);
    if (idx < 0) {
        _values.erase(_values.begin() + idx + 1, _values.end());
        _datetimes.erase(_datetimes.begin() + idx + 1, _datetimes.end());
    }
    return;
}

// Throws OutOfBoundsException
template <class T>
T TimeSeries<T>::valueAt(const chrono::date_time& dt) const
{

    if (isEmpty()) throw RunTimeException("empty timeseries");
    if (dt < firstDateTime()) throw RunTimeException("datetime before first requested");
    if (contains(dt)) return value(dt);
    return _values[indexOfLastBefore(dt)];
}

template <class T>
TimeSeries<T> TimeSeries<T>::select(const chrono::date_time& t1, const chrono::date_time& t2) const
{
    TimeSeries<T> ts;
    for (unsigned int i = 0; i < size(); i++)
    {
        if ((_datetimes[i] >= t1) && (_datetimes[i] <= t2)) ts.insert(_datetimes[i], _values[i]);
        if (_datetimes[i] > t2) break;
    }
    return ts;
}

// TODO very slow implementation since we are looking up the index of each element, but for now let's leave it at this...
template <class T>
TimeSeries<T> TimeSeries<T>::select(const chrono::date_time& t1, const chrono::date_time& t2, std::chrono::seconds step) const
{
    TimeSeries<T> ts;

    for (chrono::date_time t = t1; t <= t2; t += step)
    {
        int i = index(t);
        if (i < 0)
            ts.insert(t, _missing_value);
        else
            ts.insert(t, _values[i]);
    }

    return ts;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const TimeSeries<T>& ts)
{
    if (ts.isEmpty()) return os;
    for (unsigned int i = 0; i < ts.size(); i++)
    {
        os << "[" << ts.datetime(i) << "] " << ts.value(i) << std::endl;
    };
    return os;
};

} /* namespace OPAQ */
