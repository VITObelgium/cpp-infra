/*
 * H5ForecastStore.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Hdf5Buffer.h"

namespace OPAQ {

LOGGER_DEF(Hdf5Buffer);

// some string constants

const std::string Hdf5Buffer::BASETIME_DATASET_NAME("basetime");

const std::string Hdf5Buffer::START_DATE_NAME("start_date");
const std::string Hdf5Buffer::FORECAST_DATASET_NAME("fc_value");
const std::string Hdf5Buffer::DIMENSIONS_NAME("dimensions");
const std::string Hdf5Buffer::DIMENSIONS( "model x station x baseTime x fcHorizon" );
const std::string Hdf5Buffer::DESCRIPTION_NAME("description");
const std::string Hdf5Buffer::DESCRIPTION("OPAQ forecasts");
const std::string Hdf5Buffer::MODELS_DATASET_NAME("models");
const std::string Hdf5Buffer::STATION_DATASET_NAME("stations");

// set version information for this H5 store plugin
const std::string Hdf5BufferVersion( "0.1" );

Hdf5Buffer::Hdf5Buffer() : 
  ForecastBuffer() {

  // Tell the hdf5 lib not to print error messages: we will handle them properly ourselves
  H5::Exception::dontPrint();
  
  _h5file      = NULL;
  _noData      = -9999;
  _configured  = false;
  _baseTimeSet = false;
  _offset      = 0;
}

Hdf5Buffer::~Hdf5Buffer() {
  _closeFile();
}

void Hdf5Buffer::configure(TiXmlElement * configuration)
  throw (BadConfigurationException) {
    
  if (_configured) _closeFile();
  
  if ( ! configuration )
    throw NullPointerException("No configuration element given for Hdf5Buffer...");


  // 1. parse filename
  TiXmlElement * fileEl = configuration->FirstChildElement("filename");
  if (!fileEl)
    throw BadConfigurationException("filename element not found");
  _filename = fileEl->GetText();
  
  // 2. parse start date
  TiXmlElement * offsetEl = configuration->FirstChildElement("offset");
  if (!offsetEl)
    throw BadConfigurationException("offset element not found");
  _offset = atoi(offsetEl->GetText());

  // 3. need to specify the time interval for which to store these values...
  //    this has to be generic, the baseTime resolution can be different from the
  //    forecast time resolution...
  TiXmlElement * baseResEl = configuration->FirstChildElement("basetime_resolution");
  if ( ! baseResEl ) {
	  _baseTimeResolution = TimeInterval( 24, TimeInterval::Hours );
  } else {
	  _baseTimeResolution = TimeInterval( atoi(baseResEl->GetText()), TimeInterval::Hours );
  }

  TiXmlElement * fcResEl = configuration->FirstChildElement("fctime_resolution");
  if ( ! fcResEl ) {
	  _fcTimeResolution = TimeInterval( 24, TimeInterval::Hours );
  } else {
  	  _fcTimeResolution = TimeInterval( atoi(fcResEl->GetText()), TimeInterval::Hours );
  }

  
  _createOrOpenFile();
  _configured = true;
  
}

void Hdf5Buffer::setNoData(double noData) {
  this->_noData = noData;
}

double Hdf5Buffer::getNoData() {
  // an alternative would be to read back the no data value from one of the 
  // datasets in the data, but the question is then... which one ?
  
  // _checkFullyConfigured();
  // _dataSet.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE, &out);
  return _noData;
}


/*
void Hdf5Buffer::setBaseTime(const DateTime & baseTime)
  throw (BadConfigurationException) {

  this->_baseTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_DAY);
  if (_configured) _calcStartDateAndOpenFile();
  _baseTimeSet = true;

  return;
}
*/

/* =================================================================================
   Set the values of the given forecast for the given baseTime
   ============================================================================== */  
void Hdf5Buffer::setValues( const DateTime &baseTime,
	 		  	     	  	const OPAQ::TimeSeries<double>& forecast,
	 						const std::string& stationId,
	 						const std::string& pollutantId,
	 						OPAQ::Aggregation::Type aggr ) {

	// -- check whether we have our configuration
	if ( ! _configured  ) throw NotConfiguredException("Not fully configured");
  
	// -- do nothing if no values are given
	if ( forecast.size() == 0 ) return;

	// -- check whether the forecast array is sequential
	for( unsigned int i = 0; i<forecast.size(); i++ ) {
		if ( forecast.datetime(i) < baseTime ) throw RunTimeException( "Expecting forecasts in the future" );
		TimeInterval fc_hor = OPAQ::TimeInterval( baseTime, forecast.datetime(i) );
		if ( fc_hor != ( i * _fcTimeResolution ) ) throw RunTimeException( "Provided values should be ordered & spaced by time resolution" );
	}

	// -- locate or create the groups in the file, 4 levels

	// create 3D datasets : model x station x baseTime x fchorizon
	// grouped by pollutant and aggregation time
	// that way we get some structure, but the retrieval of a.g. forecasted values across the stations remains very efficient !


	//    1. group for the pollutant
	H5::Group grpPol, grpAggr;
	try {
		grpPol = _h5file->openGroup( pollutantId );
	} catch ( H5::Exception &err ) {
		try {
			grpPol = _h5file->createGroup( pollutantId );
		} catch ( H5::Exception &err2 ) {
			throw RunTimeException("Unable to create " + pollutantId + " group in H5 forecast store");
		}
	}

	//    2. group for the aggregation time
	try {
		grpAggr = grpPol.openGroup( OPAQ::Aggregation::getName( aggr ) );
	} catch ( H5::Exception &err ) {
		try {
			grpAggr = grpPol.createGroup( OPAQ::Aggregation::getName( aggr ) );
		} catch( H5::Exception &err2 ) {
			throw RunTimeException("Unable to create " + OPAQ::Aggregation::getName( aggr ) + " group for "
					+ pollutantId + " in H5 forecast store");
		}
	}

	// -- open or create data set...
	H5::DataSet dsVals;      // H5 dataset for the forecast values

	// meta datasets...
	H5::DataSet dsStations;  // H5 dataset for the stations names
	H5::DataSet dsModels;    // H5 dataset for the model names

	// -- the start time from the data file
	DateTime startTime;
	try {

		// open the dataset
		dsVals = grpAggr.openDataSet( FORECAST_DATASET_NAME );

		// retrieve startTime from the stored dataset in this group
		startTime = OPAQ::DateTime( Hdf5Tools::readStringAttribute( dsVals, START_DATE_NAME ) );

		// TODO handle some more errors here...

		if (startTime > baseTime )
			throw BadConfigurationException( "baseTime is before start date in the dataset" );

		// open datasets for models & stations
		dsModels   = grpAggr.openDataSet( MODELS_DATASET_NAME );
		dsStations = grpAggr.openDataSet( STATION_DATASET_NAME );

	} catch ( H5::GroupIException & err ) {

		// --------------------------------------------------------
		// 1. create the dataset at the grpAggr location
		//    dimensions : model x station x baseTime x fchorizon
		// --------------------------------------------------------
		hsize_t dims[4] = { 0, 0, 0, 0 };
		hsize_t maxdims[4] = { H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED };
		H5::DataSpace dataSpace(4, dims, maxdims);

		// create parameters for the dataset
		H5::DSetCreatPropList cparms;
		hsize_t chunks[4] = { 1, 10, 10, 1 };
		cparms.setChunk(4, chunks);
		cparms.setFillValue(H5::PredType::NATIVE_DOUBLE, &_noData); // set no data value,n HDF5 handles this automatically

		// create data set
		dsVals = grpAggr.createDataSet( FORECAST_DATASET_NAME, H5::PredType::NATIVE_DOUBLE, dataSpace, cparms);

		// we're creating a new file, so the baseTime given here is the start time of the dataset
		// we take the hour as a basic measure, flooring it to the hour !! this allows alternative
		// time resolutions later on...
		startTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_HOUR);

		Hdf5Tools::createStringAttribute(dsVals, START_DATE_NAME, startTime.toString() );
		Hdf5Tools::createStringAttribute(dsVals, DIMENSIONS_NAME, DIMENSIONS);
		Hdf5Tools::createStringAttribute(dsVals, DESCRIPTION_NAME, DESCRIPTION);

		// --------------------------------------------------------
		// 2. create the datasets with meta data: models & stations
		// --------------------------------------------------------
		hsize_t dims2[1] = { 0 };
		hsize_t maxdims2[1] = { H5S_UNLIMITED };
		H5::DataSpace dataSpace2(1, dims2, maxdims2);
		H5::DSetCreatPropList cparms2;
		hsize_t chunks2[1] = { 5 };
		cparms2.setChunk(1, chunks2);
		std::string noData("n/a");
		cparms2.setFillValue(Hdf5Tools::stringType, &noData); // set no data value

		dsModels   = grpAggr.createDataSet( MODELS_DATASET_NAME, Hdf5Tools::stringType, dataSpace2, cparms2);
		dsStations = grpAggr.createDataSet( STATION_DATASET_NAME, Hdf5Tools::stringType, dataSpace2, cparms2);

	}

	// -- now, store the data

	// get the indices
	unsigned int modelIndex = Hdf5Tools::getIndexInStringDataSet( dsModels, _currentModel, true);
	unsigned int stationIndex = Hdf5Tools::getIndexInStringDataSet( dsStations, stationId, true);
	unsigned int dateIndex = OPAQ::TimeInterval( startTime, baseTime ).getSeconds() / _baseTimeResolution.getSeconds(); // integer division... should be ok !
	unsigned int fhIndex = OPAQ::TimeInterval( forecast.firstDateTime(), baseTime ).getSeconds() / _fcTimeResolution.getSeconds();

	//DEBUG !!
	std::cout << "modelIndex= " << modelIndex << ", stationIndex= " << stationIndex << ", dateIndex=" << dateIndex << ", fcIndex = " << fhIndex << std::endl;


	// -- store data
	// a. get data space
	H5::DataSpace space = dsVals.getSpace();

	// b. get current size
	hsize_t size[4];
	space.getSimpleExtentDims(size, NULL);
	bool extend = false;
	if ( modelIndex >= size[0] ) {
		size[0] = modelIndex + 1;
		extend = true;
	}
	if (stationIndex >= size[1]) {
		size[1] = stationIndex + 1;
		extend = true;
	}
	if (dateIndex >= size[2]) {
		size[2] = dateIndex + 1;
		extend = true;
	}
	if (fhIndex + forecast.size() > size[3]) {
		size[3] = fhIndex + forecast.size();
		extend = true;
	}
	// c. extend data set if required
	if (extend) {
		dsVals.extend(size);
		space.close();
		space = dsVals.getSpace();
	}
	// d. select data set hyperslab
	hsize_t count[4] = { 1, 1, 1, forecast.size() }; // TODO: set chunk size > 1 in fh dim?
	hsize_t offset[4] = { modelIndex, stationIndex, dateIndex, fhIndex };
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// e. write data to the hyperslab
	H5::DataSpace writeMemSpace(4, count);
	double fileNoData;
	dsVals.getCreatePlist().getFillValue( H5::PredType::NATIVE_DOUBLE, &fileNoData );
	if ( _noData == fileNoData ) {
		// nodata placeholder in file equals the one used in the values vector
		dsVals.write( forecast.values().data(), H5::PredType::NATIVE_DOUBLE, writeMemSpace, space);
	} else {
		// nodata placeholder in file is different from the one used in the values vector
		double newValues[count[3]];
		for (unsigned int i = 0; i < forecast.size(); i++) {
			newValues[i] = forecast.value(i) == _noData ? fileNoData : forecast.value(i);
		}
		_dataSet.write(&newValues[0], H5::PredType::NATIVE_DOUBLE, writeMemSpace, space);
	}
	space.close();
	dsVals.flush(H5F_SCOPE_GLOBAL);



	/*

	TODO : store the basetimes as well for convenience...

	// f. write basetimes to hyperslab
	H5::DataSpace spaceBaseTimes = dataBaseTimes.getSpace();

  // b. get current size
  hsize_t size1[1];
  spaceBaseTimes.getSimpleExtentDims(size, NULL);
  extend = false;
  if (dateIndex >= size[0]) {
    size[0] = dateIndex + 1;
    extend = true;
  }
  // c. extend data set if required
  if (extend) {
    dataBaseTimes.extend(size);
    spaceBaseTimes.close();
    spaceBaseTimes = dataBaseTimes.getSpace();
  }

  // d. select data set hyperslab
  hsize_t count1[1]  = { 1 }; // TODO: set chunk size > 1 in fh dim?
  hsize_t offset1[1] = { dateIndex };
  spaceBaseTimes.selectHyperslab(H5S_SELECT_SET, count1, offset1);
  
  // e. write data to the hyperslab
  std::stringstream ss;
  ss << _baseTime;
  std::string writeBuffer[1]; writeBuffer[0] = ss.str();
  H5::DataSpace writeMemSpace1(1, count);
  dataBaseTimes.write( writeBuffer, Hdf5Tools::stringType, writeMemSpace1, spaceBaseTimes);

  spaceBaseTimes.close();
*/

  // -- close datasets and groups
  dsVals.close();
  grpPol.close();
  grpAggr.close();
}
  
  
void Hdf5Buffer::_closeFile() {
  if (_h5file != NULL) {
    _h5file->close();
    delete _h5file;
  }
}
  
void Hdf5Buffer::_checkFullyConfigured() {
  if ( ! _configured  ) throw NotConfiguredException("Not fully configured");
}


void Hdf5Buffer::_checkIfExistsAndOpen(){

	if (_h5file == NULL) {
		if (FileTools::exists(_filename))
			_openFile(_filename);
		else {
			std::stringstream ss;
			ss << "Cannot read from non existing " << _filename;
			throw RunTimeException(ss.str());
		}
	}

	return;
}
 
void Hdf5Buffer::_createOrOpenFile()
	throw (BadConfigurationException) {

	if (_h5file == NULL) { // don't open a new file if another base time is set

		if (FileTools::exists(_filename)) {
			_openFile(_filename);
		} else {
			_createFile(_filename);
		}
	}

	return;
}
  
void Hdf5Buffer::_createFile(const std::string & filename) {

  // Create a new file if not exists file
  try {
    _h5file = new H5::H5File(filename, H5F_ACC_TRUNC);
  } catch (H5::FileIException & e) {
    std::stringstream ss;
    ss << "Failed to create file " << filename
       << " (disk full? no write permissions?)";
    throw BadConfigurationException(ss.str());
  }

  // Create OPAQ file creation tag
  try {
    H5::Group rootGroup = _h5file->openGroup( "/" );

    H5::Attribute att;
    H5::DataSpace att_space(H5S_SCALAR);

    att = rootGroup.createAttribute( "HDF5BUFFER_VERSION", Hdf5Tools::stringType, att_space);
    att.write( Hdf5Tools::stringType, Hdf5BufferVersion );
    att.close();

    rootGroup.close();

  } catch ( H5::FileIException &e ) {
    throw BadConfigurationException( "Failed to initialise OPAQ HDF5 forecast buffer..." );
  }

}


void Hdf5Buffer::_openFile(const std::string &filename)
  throw (BadConfigurationException) {

  // 1. open file
  try {
    _h5file = new H5::H5File(filename, H5F_ACC_RDWR);
  } catch (H5::FileIException & e) {
    std::stringstream ss;
    ss << "Failed to open file " << filename
       << " (is it a valid HDF5 file?)";
    throw BadConfigurationException(ss.str());
  }

}
  

TimeInterval Hdf5Buffer::getTimeResolution(){
  return _fcTimeResolution; // return time resolution of the forecasts
}

TimeInterval Hdf5Buffer::getBaseTimeResolution(){
	return _baseTimeResolution;
}

/*
std::pair<const TimeInterval, const TimeInterval> Hdf5Buffer::getRange() {
  return getRange(ForecastHorizon(0));
}

std::pair<const TimeInterval, const TimeInterval> Hdf5Buffer::getRange(
		       const ForecastHorizon & forecastHorizon) {


  _checkFullyConfigured();
  DateTime begin = _startDate;
  begin.addSeconds(forecastHorizon.getSeconds());
  DateTime end = begin;
   end.addDays(size() - 1);
  TimeInterval beginOffset(_baseTime, begin);
  TimeInterval endOffset(_baseTime, end);
  return std::pair<const TimeInterval, const TimeInterval>(beginOffset,
							   endOffset);

}
  */
/*
unsigned int Hdf5Buffer::size() {
  //_checkFullyConfigured();
  //return Hdf5Tools::getDataSetSize(_dataSet, 2);
  std::cout << "IMPLEMENT MEEEEE !!! " << std::endl;
  return 0;
}
*/

OPAQ::TimeSeries<double> Hdf5Buffer::getValues( const DateTime& t1,
		const DateTime& t2, const std::string& stationId, const std::string& pollutantId,
		OPAQ::Aggregation::Type aggr ) {

	// what to return here ??
	throw RunTimeException( "not sure what to return here, need extra information ???" );

	OPAQ::TimeSeries<double> out;
	return out;
}

OPAQ::TimeSeries<double> Hdf5Buffer::getValues( const DateTime &baseTime,
		const std::vector<OPAQ::TimeInterval>& fc_hor, const std::string& stationId,
		const std::string& pollutantId, OPAQ::Aggregation::Type aggr ) {

	throw RunTimeException( "IMPLEMENT ME !!" );

	OPAQ::TimeSeries<double> out;
	return out;
}

OPAQ::TimeSeries<double> Hdf5Buffer::getValues( const OPAQ::TimeInterval fc_hor,
		const DateTime &fcTime1, const DateTime &fcTime2,
		const std::string& stationId, const std::string& pollutantId,
		OPAQ::Aggregation::Type aggr ) {

	throw RunTimeException( "IMPLEMENT ME !!" );

	OPAQ::TimeSeries<double> out;
	return out;
}


/*
std::vector<double> Hdf5Buffer::getValues(const std::string  & modelName, 
					  const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string  & parameter,
					  const std::string  & station,
					  const ForecastHorizon & forecastHorizon ) {
  _checkFullyConfigured();
  _checkIfExistsAndOpen();

  // 1. validate forecastHorizon
  long days = forecastHorizon.getHours();
  if (days % 24 != 0)
    throw RunTimeException( "Forecast horizon must be a multiple of 24 hours" );
  days /= 24;

  // a. calculate begin and end from given intervals and the base time
  DateTime begin = _baseTime;
  begin.addSeconds(beginOffset.getSeconds());
  DateTime end = _baseTime;
  end.addSeconds(endOffset.getSeconds());
  
  // a. first round them to the nearest day boundaries
  DateTime myBegin = DateTimeTools::floor(begin, DateTimeTools::FIELD_DAY);
  DateTime myEnd = DateTimeTools::ceil(end, DateTimeTools::FIELD_DAY);

  hsize_t nvals = TimeInterval( myBegin, myEnd ).getDays() + 1;

  // get indices for basetime and forecasthorizon
  hsize_t btIndex = TimeInterval(_startDate, myBegin).getDays();
  hsize_t fhIndex = days;

  // initialize the output array with the number of requested values
  std::vector<double> out(nvals);
  for ( unsigned int i = 0; i < nvals; i ++ ) out[i] = getNoData();

  H5::DataSet dset; 
  H5::Group grpStation, grpParameter, grpModel;
  // now open groups etc...
  try {
    grpStation   = _h5file->openGroup( station );
    grpParameter = grpStation.openGroup( parameter );
    grpModel     = grpParameter.openGroup( modelName );
    dset         = grpModel.openDataSet( "fc_value" );

  } catch ( H5::Exception & err)  {
    logger->trace( "no fc_value dataset or " + station + ", " + parameter + ", " + modelName + " present" );
    throw NotAvailableException( "no fc_value dataset or " + station + ", " + parameter 
				 + ", " + modelName + " present" ); 
  }    

  // now get the size of the dataset in the buffer
  hsize_t btSize = Hdf5Tools::getDataSetSize( dset, 0 );
  hsize_t fhSize = Hdf5Tools::getDataSetSize( dset, 1 );
  
  // the forecast horizon is present in the datafile
  if ( fhIndex >= 0 && fhIndex < fhSize ) {
    
    hsize_t dataStartIndex = btIndex < 0 ? 0 : btIndex;
    hsize_t dataEndIndex   = btIndex + nvals - 1;

    if ( dataStartIndex < btSize && dataEndIndex >= 0 ) {
      // only need to fill if the selected hyperslab lies within the data set
      if (dataEndIndex >= btSize) dataEndIndex = btSize - 1;
      
      hsize_t dataCount     = dataEndIndex - dataStartIndex + 1;
      hsize_t outStartIndex = btIndex < 0 ? -btIndex : 0;
      // select hyperslabs and fetch the data
      H5::DataSpace space = dset.getSpace();
      hsize_t dc[2] = { dataCount, 1 };
      hsize_t doffset[2] = { dataStartIndex, fhIndex };
      space.selectHyperslab(H5S_SELECT_SET, dc, doffset);
      hsize_t mc[1] = { nvals };
      hsize_t moffset[1] = { outStartIndex };
      H5::DataSpace memSpace(1, mc);
      mc[0] = dataCount;
      memSpace.selectHyperslab(H5S_SELECT_SET, mc, moffset);


      dset.read( &out[0], H5::PredType::NATIVE_DOUBLE, memSpace, space );

      space.close();
    }


  } else throw RunTimeException( "Requested forecast horizon index not in HDF5 buffer" );
  


  // TODO: put in a check on the basetimes
  

  
  return out;
}
 */

  // is the get values for an interpolation model --> get values for all the stations...
/*
std::vector<double> Hdf5Buffer::getValues(const std::string & parameter,
					  const TimeInterval & offset,
					  const ForecastHorizon & forecastHorizon ) {

  if ( _aqNet == NULL) 
    throw RunTimeException("no AQ network provider set, cannot provide values for stations");
  std::vector<double> out;
  Pollutant * pol = Config::PollutantManager::getInstance()->find(parameter);
  AQNetwork * net = _aqNet->getAQNetwork();
  std::vector<Station *> * stations = &(net->getStations());
  std::vector<Station *>::iterator it = stations->begin();
  while (it != stations->end()) {
    Station * station = *it++;
    if (pol == NULL || AQNetworkTools::stationHasPollutant(station, *pol)) {
      //
      // the parameter is not a pollutant
      // or
      // the parameter is a pollutant and it is provided by the station
      //
      // in both cases, we need to fetch the data from the file
      //
      out.push_back( getValues(offset, offset, parameter, station->getName(),
			       forecastHorizon).front() );
    } else {
      //
      // the parameter denotes a pollutant, but it is not provided by the station
      //
      out.push_back(getNoData());
    }
  }
  return out;
  // TODO: can we implement this more efficiently by directly querying the h5 file?
  
  
}

*/

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::Hdf5Buffer);
