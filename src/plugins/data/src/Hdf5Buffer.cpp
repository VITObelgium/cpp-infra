/*
 * H5ForecastStore.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include <opaq/common.h>

#include "Hdf5Buffer.h"

namespace OPAQ {

LOGGER_DEF(Hdf5Buffer);

// some string constants
const std::string Hdf5Buffer::START_DATE_NAME("START_DATE");
const std::string Hdf5Buffer::FORECAST_DATASET_NAME("fc_value");
const std::string Hdf5Buffer::BASETIME_DATASET_NAME("basetime");

// set version information for this H5 store plugin
const std::string Hdf5BufferVersion("0.1");

Hdf5Buffer::Hdf5Buffer() : 
  DataBuffer() {

  // tell the hdf5 lib not to print error messages: we will handle them properly ourselves
  H5::Exception::dontPrint();
  
  _h5file      = NULL;
  _noData      = -9999;
  _configured  = false;
  _baseTimeSet = false;
  _aqNet       = NULL;
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

  if (_baseTimeSet) _calcStartDateAndOpenFile();
  
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

double Hdf5Buffer::getNoData(const std::string & id){
  return getNoData();
}



void Hdf5Buffer::setBaseTime(const DateTime & baseTime)
  throw (BadConfigurationException) {

  this->_baseTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_DAY);
  if (_configured) _calcStartDateAndOpenFile();
  _baseTimeSet = true;

  return;
}


/* =================================================================================
   Set the values...
   ============================================================================== */  
void Hdf5Buffer::setValues( const std::string & modelName,
			    const std::vector<double> & values,
			    const std::vector<ForecastHorizon> & forecastHorizons,
			    const std::string & parameter, const std::string & station) {

  // -- check whether we have our configuration
  _checkFullyConfigured();
  
  // -- do nothing if no values are given
  if (values.size() == 0) return; 
  
  // -- check the forecast horizons given.. 
  if (values.size() != forecastHorizons.size())
    throw RunTimeException("number of values != number of forecastHorizons");
  std::vector<ForecastHorizon>::const_iterator it = forecastHorizons.begin();
  long previousHours = 0;
  while (it != forecastHorizons.end()) {
    long hours = it->getHours();
    if (hours % 24 != 0)
      throw RunTimeException("Not all forecast horizons lie on exact day boundaries");
    if (it != forecastHorizons.begin() && hours - previousHours != 24) {
      throw RunTimeException("Forecast horizons must be consecutive days (24 hours)");
    }
    previousHours = hours;
    it++;
  }
  
  // -- locate or create the groups in the file, 3 levels
  //    1. group for the station 
  H5::Group grpStation, grpParameter, grpModel;
  try {
    grpStation = _h5file->openGroup( station );
  } catch ( H5::Exception &err ) {
    try {
      grpStation = _h5file->createGroup( station );
    } catch ( H5::Exception &err2 ) {
      throw RunTimeException("Unable to create " + station + " group in H5 forecast store");
    }
  }

  //    2. group for the parameter
  try {
    grpParameter = grpStation.openGroup( parameter );
  } catch ( H5::Exception &err ) {
    try {
      grpParameter = grpStation.createGroup( parameter );
    } catch( H5::Exception &err2 ) {
      throw RunTimeException("Unable to create " + parameter + " group for " 
			     + station + " in H5 forecast store");
    }
  }

  //    3. group corresponding with the model
  try {
    grpModel = grpParameter.openGroup( modelName );
  } catch ( H5::Exception &err ) {
    try {
      grpModel = grpParameter.createGroup( modelName );
    } catch ( H5::Exception &err2 ) {
      throw RunTimeException("Unable to create " + modelName + " group for " 
			     + parameter + ", " + station + " in H5 forecast store");
    }
  }
  
  // -- open or create data set...
  H5::DataSet dataVals;      // H5 dataset for the forecast values
  H5::DataSet dataBaseTimes; // H5 dataset for the basetimes
  try {

    dataVals       = grpModel.openDataSet( FORECAST_DATASET_NAME );
    dataBaseTimes  = grpModel.openDataSet( BASETIME_DATASET_NAME );

    // TODO: add some metadata...

  } catch ( H5::GroupIException & err ) {

    // Create the forecast values dataset...
    hsize_t dims[2]    = { 0, 0 };
    hsize_t maxdims[2] = { H5S_UNLIMITED, H5S_UNLIMITED };
    H5::DataSpace space(2, dims, maxdims);

    H5::DSetCreatPropList cparms;
    hsize_t chunks[2] = { 1, 5 };
    cparms.setChunk(2, chunks);
    // set nodata value for filling up the dataset
    cparms.setFillValue(H5::PredType::NATIVE_DOUBLE, &_noData);
    
    try {
      dataVals = grpModel.createDataSet( FORECAST_DATASET_NAME, H5::PredType::NATIVE_DOUBLE, space, cparms );
    } catch ( H5::Exception & err2 ) {
      throw RunTimeException( "Error creating new forecast dataset in HDF5 file... " );
    }
    space.close();

    // Create the basetimes values dataset...
    hsize_t dims1[1] = { 0 };
    hsize_t maxdims1[1] = { H5S_UNLIMITED };
    H5::DataSpace space1(1, dims1, maxdims1);

    H5::DSetCreatPropList cparms1;
    hsize_t chunks1[1] = { 1 };
    cparms1.setChunk(1, chunks1);
    // set nodata value
    std::string noData("n/a");
    cparms1.setFillValue( Hdf5Tools::stringType, &noData );

    try {
      dataBaseTimes  = grpModel.createDataSet( BASETIME_DATASET_NAME, Hdf5Tools::stringType, space1, cparms1 );
    } catch ( H5::Exception & err2) {
      throw RunTimeException( "Error creating new basetime dataset in HDF5 file..." ); 
    }
    space1.close();

  } // end of try block where we create/open the datasets


  // -- now, store the data

  // get the indices
  unsigned int dateIndex = TimeInterval( _startDate, _baseTime ).getDays();
  unsigned int fhIndex   = forecastHorizons.begin()->getHours() / 24;

  // a. get data space
  H5::DataSpace spaceVals = dataVals.getSpace();

  // b. get current size
  hsize_t size[2];
  spaceVals.getSimpleExtentDims(size, NULL);
  bool extend = false;
  if (dateIndex >= size[0]) {
    size[0] = dateIndex + 1;
    extend = true;
  }
  if (fhIndex + forecastHorizons.size() > size[1]) {
    size[1] = fhIndex + forecastHorizons.size();
    extend = true;
  }

  // c. extend data set if required
  if (extend) {
    dataVals.extend(size);
    spaceVals.close();
    spaceVals = dataVals.getSpace();
  }

  // d. select data set hyperslab
  hsize_t count[2]  = { 1, forecastHorizons.size() }; // TODO: set chunk size > 1 in fh dim?
  hsize_t offset[2] = { dateIndex, fhIndex };
  spaceVals.selectHyperslab(H5S_SELECT_SET, count, offset);
  
  // e. write forecast data to the hyperslab
  H5::DataSpace writeMemSpace(2, count);
  double fileNoData;
  dataVals.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE,&fileNoData);
  if (_noData == fileNoData) {
    // nodata placeholder in file equals the one used in the values vector
    dataVals.write( &values[0], H5::PredType::NATIVE_DOUBLE, writeMemSpace, spaceVals);
  } else {
    // nodata placeholder in file is different from the one used in the values vector
    double newValues[count[1]];
    for (unsigned int i = 0; i < values.size(); i++) {
      newValues[i] = values[i] == _noData ? fileNoData : values[i];
    }
    dataVals.write(&newValues[0], H5::PredType::NATIVE_DOUBLE, writeMemSpace, spaceVals);
  }
  spaceVals.close();


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

  // -- close datasets and groups
  dataVals.close();
  dataBaseTimes.close();

  grpModel.close();
  grpParameter.close();
  grpStation.close();
}
  
void Hdf5Buffer::setValues(const std::string &modelName,
			   const std::vector<double> & values,
			   const std::string & id,
			   const ForecastHorizon & forecastHorizon) {
  throw RunTimeException("Cannot use Hdf5Buffer to store mappings.");
}
  
void Hdf5Buffer::_closeFile() {
  if (_h5file != NULL) {
    _h5file->close();
    delete _h5file;
  }
}
  
void Hdf5Buffer::_checkFullyConfigured() {
  if (!_configured || !_baseTimeSet)
    throw NotConfiguredException("Not fully configured");
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
}
 
void Hdf5Buffer::_calcStartDateAndOpenFile()
  throw (BadConfigurationException) {
  if (_h5file == NULL) { // don't open a new file if another base time is set
    _startDate = _baseTime;
    _startDate.addDays(_offset); // usually a negative offset
    if (_startDate > _baseTime)
      throw BadConfigurationException("start date cannot be after base time");
    
    if (FileTools::exists(_filename)) {
      _openFile(_filename);
    } else {
      _createFile(_filename);
    }
  }
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

  // add data set attributes
  std::stringstream ss;
  ss << _startDate;
  
  // Create OPAQ file creation tag
  try {
    H5::Group rootGroup = _h5file->openGroup( "/" );

    H5::Attribute att;
    H5::DataSpace att_space(H5S_SCALAR);

    att = rootGroup.createAttribute( START_DATE_NAME, Hdf5Tools::stringType, att_space);
    att.write( Hdf5Tools::stringType, ss.str().substr(0,10) );
    att.close();    

    att = rootGroup.createAttribute( "OPAQ_VERSION", Hdf5Tools::stringType, att_space);
    att.write( Hdf5Tools::stringType, OPAQ::Version );
    att.close();

    att = rootGroup.createAttribute( "H5FORECASTSTORE_VERSION", Hdf5Tools::stringType, att_space);
    att.write( Hdf5Tools::stringType, Hdf5BufferVersion );
    att.close();

    rootGroup.close();

  } catch ( H5::FileIException &e ) {
    throw BadConfigurationException( "Failed to initialise OPAQ HDF5 forecast store..." );
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
  
  // 2. check file format --> presence of START_DATE 
  try {
    std::string dateStr;
    H5::Group rootGroup = _h5file->openGroup( "/" );
    H5::Attribute att = rootGroup.openAttribute( START_DATE_NAME );
    att.read( Hdf5Tools::stringType, dateStr );
    att.close();
    rootGroup.close();

    DateTime fromFile = DateTimeTools::parseDate(dateStr);
    if (fromFile > _startDate) {
      std::stringstream ss;
      ss << "start date is before the first date in file " << filename;
      throw BadConfigurationException(ss.str());
    }
    _startDate = fromFile;
  } catch (H5::AttributeIException & e) {
    std::stringstream ss;
    ss << "Failed to get start date from file " << filename
       << " (file corrupted?)";
    throw BadConfigurationException(ss.str());
  } catch (ParseException & e) {
    std::stringstream ss;
    ss << "Failed to parse date in start_date attribute in file "
       << filename << " (file corrupted?)";
    throw BadConfigurationException(ss.str());
  }

}
  
void Hdf5Buffer::setAQNetworkProvider(AQNetworkProvider * aqNetworkProvider) {
  _aqNet = aqNetworkProvider;
}
  
TimeInterval Hdf5Buffer::getTimeResolution(){
  return OPAQ::TimeInterval( 86400 ); // 1 day at the moment
}

std::pair<const TimeInterval, const TimeInterval> Hdf5Buffer::getRange() {
  return getRange(ForecastHorizon(0));
}

std::pair<const TimeInterval, const TimeInterval> Hdf5Buffer::getRange(
		       const ForecastHorizon & forecastHorizon) {

  /*
  _checkFullyConfigured();
  DateTime begin = _startDate;
  begin.addSeconds(forecastHorizon.getSeconds());
  DateTime end = begin;
   end.addDays(size() - 1);
  TimeInterval beginOffset(_baseTime, begin);
  TimeInterval endOffset(_baseTime, end);
  return std::pair<const TimeInterval, const TimeInterval>(beginOffset,
							   endOffset);
  */
}

unsigned int Hdf5Buffer::size() {
  //_checkFullyConfigured();
  //return Hdf5Tools::getDataSetSize(_dataSet, 2);
  std::cout << "IMPLEMENT MEEEEE !!! " << std::endl;
  return 0;
}

std::vector<double> Hdf5Buffer::getValues(const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & parameter,
					  const std::string & station ) {
  return getValues(beginOffset, endOffset, parameter, station, ForecastHorizon(0) );
}

std::vector<double> Hdf5Buffer::getValues(const std::string & modelName,
					  const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & parameter,
					  const std::string & station ) {
  return getValues(modelName, beginOffset, endOffset, parameter, station, ForecastHorizon(0) );
}

std::vector<double> Hdf5Buffer::getValues(const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & parameter,
					  const std::string & station,
					  const ForecastHorizon & forecastHorizon ) {
  std::vector<double> out;
  throw 
    RunTimeException("HDF5 buffer without model name (so for observations) not implemented yet");
  return out;
}

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
 

  // is the get values for an interpolation model --> get values for all the stations...
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
      /*
       * the parameter is not a pollutant
       * or
       * the parameter is a pollutant and it is provided by the station
       *
       * in both cases, we need to fetch the data from the file
       */
      out.push_back( getValues(offset, offset, parameter, station->getName(),
			       forecastHorizon).front() );
    } else {
      /*
       * the parameter denotes a pollutant, but it is not provided by the station
       */
      out.push_back(getNoData());
    }
  }
  return out;
  // TODO: can we implement this more efficiently by directly querying the h5 file?
  
  
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::Hdf5Buffer);
