/*
 * TimeSeries.h
 *
 *  Created on: Dec 18, 2015
 *      Author: bino
 */

#ifndef SRC_OPAQ_CORE_TIMESERIES_H_
#define SRC_OPAQ_CORE_TIMESERIES_H_

#include <iostream>
#include <fstream>
#include <algorithm>
#include "Exceptions.h"
#include "DateTime.h"

namespace OPAQ {

/**
 * Basic template class for OPAQ timeseries
 */
template <class T>
class TimeSeries {
public:
	TimeSeries<T>();
	TimeSeries<T>( const TimeSeries<T>& );
	TimeSeries<T> operator= ( const TimeSeries<T>& );

	~TimeSeries(){ clear(); }

	size_t size() const { return _datetimes.size(); }
	bool   isEmpty() const { return ( _datetimes.size() == 0 ); }
	bool   isConsistent( const TimeSeries<T>& t ) const {
		if ( t.size() != size() ) return false;
		for ( unsigned int i = 0; i < size(); i++ )
			if ( _datetimes[i] != t.datetimes().at(i) ) return false;
		return true;
	}

	const std::vector<T>& values( void ) const { return _values; }
	const std::vector<OPAQ::DateTime>& datetimes( void ) const { return _datetimes; }

	// ======================
	// insert functionality
	// ======================
	/**
	 * Insert elements into the timeseries, given as a datatime object and a
	 * value (of type T)
	 */
	void insert( const DateTime&, const T& ) throw( OutOfBoundsException );
	/**
	 * Merges the given timeseries into this timeseries
	 * value (of type T). When duplicate datetimestamps are present,
	 * if overwrite is true, the existing value will be overwritten otherwise
	 * the old values will be kept...
	 */
	void merge( const TimeSeries<T>& dt, bool overwrite = true );

	// ===================
	// element removal
	// ===================
	void clear() {
		_values.erase( _values.begin(), _values.end() );
		_datetimes.erase( _datetimes.begin(), _datetimes.end() );
	}
	void remove( const DateTime& );
	void removeRange( const DateTime& , const DateTime&, bool with_ends = false );
	void removeBefore( const DateTime& );
	void removeAfter( const DateTime& );

	// ===================
	// search algorithms
	// ===================

	// does the timeseries contain the given datetime
	bool contains( const DateTime& dt ) const { return std::binary_search(_datetimes.begin(), _datetimes.end(), dt ); }

	// return the first date time in the timeseries
	DateTime firstDateTime() const {
		return _datetimes.front();
	};
	// return the last date time in the timeseries
	DateTime lastDateTime() const {
		return _datetimes.back();
	};
	// return the index of the date time, -1 if not found
	int index( const DateTime& dt ) const {
		for ( unsigned int i=0; i<size(); i++ )
			if ( _datetimes[i] == dt ) return i;
		return -1; // not found
	}
	// return first index after the given datetime
	int indexOfFirstAfter( const DateTime& dt ) const {
		if ( ! dt.isValid() ) return -1;
		if ( dt >= lastDateTime() ) return -1;
		if ( dt < firstDateTime() ) return 0;
		for ( unsigned int i=0; i<size(); i ++ )
			if ( _datetimes[i] > dt ) return i;
		return -1; // not found
	}
	// return first index after the given datetime
	int indexOfLastBefore( const DateTime& dt ) const {
		if ( ! dt.isValid() ) return -1;
		if ( dt <= firstDateTime() ) return -1;
		if ( dt > lastDateTime() ) return index( lastDateTime() );
		for ( unsigned int i=0; i<size(); i ++ )
			if ( _datetimes[i] >= dt ) return i-1;
		return -1; // not found
	}
	DateTime datetime( unsigned int i ) const throw(OutOfBoundsException) {
		if ( i >= size() ) throw OPAQ::OutOfBoundsException( "index out of range" );
		return _datetimes[i];
	}
	T value( unsigned int i ) const throw(OutOfBoundsException) {
		if ( i >= size() ) throw OPAQ::OutOfBoundsException( "index out of range" );
		return _values[i];
	}
	T value( const DateTime& dt ) const throw(OutOfBoundsException) {
		if ( ! contains(dt) ) throw OPAQ::OutOfBoundsException( "datetime not found in timeseries" );
		return _values[index(dt)];
	}
	// this one returns either the value at dt if present, or the last value before the
	// given timestamp (as the at that time most recent value)
	T valueAt( const DateTime& dt ) const throw(OutOfBoundsException);

	// subset the timeseries, returns a new timeseries with the subset of the
	// object
	TimeSeries<T> select( const DateTime& t1, const DateTime& t2 ) const;


	// write the timeseries to a file
	void write( std::string fname ) {
	  std::ofstream fs;
	  fs.open( fname, std::ios::out | std::ios::trunc ); // open for output & truncate if exists
	  fs << *this;
	  fs.close();
	  return;
	}
	

protected:
	std::vector<T>        _values;
	std::vector<DateTime> _datetimes;
};

// empty constructor
template <class T>
TimeSeries<T>::TimeSeries(){}

// copy constructor
template <class T>
TimeSeries<T>::TimeSeries( const TimeSeries<T>& ts ) {
	_datetimes = std::vector<DateTime>( ts.size() );
	_values    = std::vector<T>( ts.size() );
	for ( unsigned int i=0; i<ts.size(); ++i ) {
		_values[i]     = ts.value(i);
		_datetimes[i]  = ts.datetime(i);
	}
}
// assignement operator
template <class T>
TimeSeries<T> TimeSeries<T>::operator= ( const TimeSeries<T>& ts ) {
	if ( this == &ts ) return *this;
	clear(); // clear the current object, will be set to new ts
	_datetimes = std::vector<DateTime>( ts.size() );
	_values    = std::vector<T>( ts.size() );
	for ( unsigned int i=0; i<ts.size(); ++i ) {
		_values[i]     = ts.value(i);
		_datetimes[i]  = ts.datetime(i);
	}
}

// insert function
template <class T>
void TimeSeries<T>::insert( const DateTime& dt, const T& val )
		throw( OutOfBoundsException ) {

	if ( ! dt.isValid() ) throw OPAQ::OutOfBoundsException( "invalid datetime given" );
	// if the timeseries is empty or the date is after the last date, push it back...
	if ( isEmpty() || dt > lastDateTime() ) {
		_datetimes.push_back( dt );
		_values.push_back( val );
		return;
	}
	// if the index if before the first inded
	if ( dt < firstDateTime() ) {
		_datetimes.insert( _datetimes.begin(), dt );
		_values.insert( _values.begin(), val );
		return;
	}
	// if the timeseries contains the datetime, replace the value
	if ( contains( dt ) ) {
		_values[ index(dt) ] = val;
		return;
	}
	// otherwise, lookup the first index after the date to insert
	int i = indexOfFirstAfter( dt );
	if ( i >= 0 ) {
		_datetimes.insert( _datetimes.begin()+i, dt );
		_values.insert( _values.begin()+i, val );
	}

	return;
}

template <class T>
void TimeSeries<T>::merge( const TimeSeries<T>& ts, bool overwrite ) {

	for ( unsigned int i = 0; i < ts.size(); i++ ) {
		if ( ! contains( ts.datetime(i) ) )
			insert( ts.datetime(i), ts.value(i) );
		else {
			if ( overwrite ) insert( ts.datetime(i), ts.value(i) );
		}
	}

	return;
}


// remove function
template <class T>
void TimeSeries<T>::remove( const DateTime& dt ) {
	if ( ! dt.isValid() ) return;
	int i = index( dt );
	if ( i >= 0 ) {
		_datetimes.erase( _datetimes.begin()+i );
		_values.erase( _values.begin()+i );
	}
	return;
}

template <class T>
void TimeSeries<T>::removeRange( const DateTime& dt1, const DateTime& dt2, bool with_ends ) {
	if ( ! dt1.isValid() ) return;
	if ( ! dt2.isValid() ) return;

	// whole range ?
	if ( ( dt1 < firstDateTime() ) && ( dt2 > lastDateTime() ) ) {
		clear();
		return;
	}

	int i1 = indexOfFirstAfter( dt1 );
	int i2 = indexOfLastBefore( dt2 );

	if ( i1 < 0 ) return; // first index in range to be remove is after the timeseries
	if ( i2 < 0 ) return; // last index in range to be removed is before the first element in timeseries

	_values.erase( _values.begin()+i1, _values.begin()+i2+1 );
	_datetimes.erase( _datetimes.begin()+i1, _datetimes.begin()+i2+1 );

	if ( with_ends) {
		remove( dt1 );
		remove( dt2 );
	}

	return;
}

template <class T>
void TimeSeries<T>::removeBefore( const DateTime &t ) {
	if ( ! t.isValid() ) return;
	int idx = indexOfLastBefore( t );
	if ( idx < 0 ) {
		_values.erase( _values.begin(), _values.begin()+idx+1 );
		_datetimes.erase( _datetimes.begin(), _datetimes.begin()+idx+1 );
	}
	return;
}

template <class T>
void TimeSeries<T>::removeAfter( const DateTime &t ) {
	if ( ! t.isValid() ) return;
	int idx = indexOfFirstAfter( t );
	if ( idx < 0 ) {
		_values.erase( _values.begin()+idx+1, _values.end() );
		_datetimes.erase( _datetimes.begin()+idx+1, _datetimes.end() );
	}
	return;
}

template <class T>
T TimeSeries<T>::valueAt( const DateTime& dt ) const
		throw(OutOfBoundsException) {

	if ( isEmpty() ) throw OPAQ::OutOfBoundsException( "empty timeseries" );
	if ( dt < firstDateTime() ) throw OPAQ::OutOfBoundsException( "datetime before first requested" );
	if (contains(dt) ) return value(dt);
	return _values[ indexOfLastBefore( dt ) ];
}


template <class T>
TimeSeries<T> TimeSeries<T>::select( const DateTime& t1, const DateTime& t2 ) const {
	TimeSeries<T> ts;
	for ( unsigned int i = 0; i < size(); i++ ) {
		if ( ( _datetimes[i] >= t1 ) && ( _datetimes[i] <= t2 ) ) ts.insert( _datetimes[i], _values[i] );
		if ( _datetimes[i] > t2 ) break;
	}
	return ts;
}


template <class T>
std::ostream& operator << (std::ostream& os, const TimeSeries<T>& ts ) {
   if ( ts.isEmpty() ) return os;
   for (unsigned int i=0; i<ts.size(); i++) {
      os << "[" << ts.datetime(i) << "] "  << ts.value(i)  << std::endl;
   };
   return os;
};


} /* namespace OPAQ */

#endif /* SRC_OPAQ_CORE_TIMESERIES_H_ */
