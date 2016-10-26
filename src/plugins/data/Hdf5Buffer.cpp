/*
 * H5ForecastStore.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Hdf5Buffer.h"
#include "Hdf5Tools.h"
#include "PluginRegistration.h"

#include <tinyxml.h>

namespace OPAQ
{

using namespace std::chrono_literals;

const std::string Hdf5Buffer::BASETIME_DATASET_NAME("basetime");

const std::string Hdf5Buffer::START_DATE_NAME("start_date");
const std::string Hdf5Buffer::FORECAST_DATASET_NAME("fc_value");
const std::string Hdf5Buffer::DIMENSIONS_NAME("dimensions");
const std::string Hdf5Buffer::DIMENSIONS("model x station x baseTime x fcHorizon");
const std::string Hdf5Buffer::DESCRIPTION_NAME("description");
const std::string Hdf5Buffer::DESCRIPTION("OPAQ forecasts");
const std::string Hdf5Buffer::MODELS_DATASET_NAME("models");
const std::string Hdf5Buffer::STATION_DATASET_NAME("stations");

// set version information for this H5 store plugin
const std::string Hdf5BufferVersion("0.1");

static const char* s_noData = "n/a";

Hdf5Buffer::Hdf5Buffer()
: _logger("Hdf5Buffer")
, _stringType(H5::StrType(0, H5T_VARIABLE))
{

    // Tell the hdf5 lib not to print error messages: we will handle them properly ourselves
    H5::Exception::dontPrint();

    _h5file      = NULL;
    _noData      = -9999;
    _configured  = false;
    _baseTimeSet = false;
    //_offset      = 0;
}

Hdf5Buffer::~Hdf5Buffer()
{
    _closeFile();
}

std::string Hdf5Buffer::name()
{
    return "hdf5buffer";
}

void Hdf5Buffer::configure(TiXmlElement* configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    if (_configured) _closeFile();

    if (!configuration)
        throw NullPointerException("No configuration element given for Hdf5Buffer...");

    // 1. parse filename
    TiXmlElement* fileEl = configuration->FirstChildElement("filename");
    if (!fileEl)
        throw BadConfigurationException("filename element not found");
    _filename = fileEl->GetText();

    // 2. parse start date
    /*
  TiXmlElement * offsetEl = configuration->FirstChildElement("offset");
  if (!offsetEl)
    throw BadConfigurationException("offset element not found");
  _offset = atoi(offsetEl->GetText());
   */

    // 3. need to specify the time interval for which to store these values...
    //    this has to be generic, the baseTime resolution can be different from the
    //    forecast time resolution...
    TiXmlElement* baseResEl = configuration->FirstChildElement("basetime_resolution");
    if (!baseResEl) {
        _baseTimeResolution = 24h;
    }
    else
    {
        _baseTimeResolution = std::chrono::hours(atoi(baseResEl->GetText()));
    }

    TiXmlElement* fcResEl = configuration->FirstChildElement("fctime_resolution");
    if (!fcResEl) {
        _fcTimeResolution = 24h;
    }
    else
    {
        _fcTimeResolution = std::chrono::hours(atoi(fcResEl->GetText()));
    }

    _createOrOpenFile();
    _configured = true;
}

void Hdf5Buffer::setNoData(double noData)
{
    this->_noData = noData;
}

double Hdf5Buffer::getNoData()
{
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
void Hdf5Buffer::setValues(const chrono::date_time& baseTime,
                           const TimeSeries<double>& forecast,
                           const std::string& stationId,
                           const std::string& pollutantId,
                           Aggregation::Type aggr)
{

    // -- check whether we have our configuration
    if (!_configured) throw RunTimeException("Hdf5Buffer Not fully configured");

    // -- do nothing if no values are given
    if (forecast.size() == 0) return;

    // -- check whether the forecast array is sequential
    for (unsigned int i = 0; i < forecast.size(); i++)
    {
        if (forecast.datetime(i) < baseTime) throw RunTimeException("Expecting forecasts in the future");

        auto secs = chrono::to_seconds(forecast.datetime(i) - baseTime);

        // this allows for setting forecast values one by one...
        if ((secs.count() % std::chrono::seconds(_fcTimeResolution).count()) != 0)
            throw RunTimeException("Provided values should be ordered & spaced by time resolution");
    }

    // -- locate or create the groups in the file, 4 levels

    // create 3D datasets : model x station x baseTime x fchorizon
    // grouped by pollutant and aggregation time
    // that way we get some structure, but the retrieval of a.g. forecasted values across the stations remains very efficient !

    //    1. group for the pollutant
    H5::Group grpPol, grpAggr;
    try
    {
        grpPol = _h5file->openGroup(pollutantId);
    }
    catch (const H5::Exception&)
    {
        try
        {
            grpPol = _h5file->createGroup(pollutantId);
        }
        catch (const H5::Exception&)
        {
            throw RunTimeException("Unable to create " + pollutantId + " group in H5 forecast store");
        }
    }

    //    2. group for the aggregation time
    try
    {
        grpAggr = grpPol.openGroup(OPAQ::Aggregation::getName(aggr));
    }
    catch (const H5::Exception&)
    {
        try
        {
            grpAggr = grpPol.createGroup(OPAQ::Aggregation::getName(aggr));
        }
        catch (const H5::Exception&)
        {
            throw RunTimeException("Unable to create " + OPAQ::Aggregation::getName(aggr) + " group for " + pollutantId + " in H5 forecast store");
        }
    }

    // -- open or create data set...
    H5::DataSet dsVals; // H5 dataset for the forecast values

    // meta datasets...
    H5::DataSet dsStations; // H5 dataset for the stations names
    H5::DataSet dsModels;   // H5 dataset for the model names

    // -- the start time from the data file
    chrono::date_time startTime;
    try
    {

        // open the dataset
        dsVals = grpAggr.openDataSet(FORECAST_DATASET_NAME);

        // retrieve startTime from the stored dataset in this group
        startTime = chrono::from_date_string(Hdf5Tools::readStringAttribute(dsVals, START_DATE_NAME));

        if (startTime > baseTime)
            throw BadConfigurationException("baseTime is before start date in the dataset");

        // open datasets for models & stations
        dsModels   = grpAggr.openDataSet(MODELS_DATASET_NAME);
        dsStations = grpAggr.openDataSet(STATION_DATASET_NAME);
    }
    catch (const H5::Exception&)
    {
        try
        {
            // --------------------------------------------------------
            // 1. create the dataset at the grpAggr location
            //    dimensions : model x station x baseTime x fchorizon
            // --------------------------------------------------------
            hsize_t dims[4]    = {0, 0, 0, 0};
            hsize_t maxdims[4] = {H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED};
            H5::DataSpace dataSpace(4, dims, maxdims);

            // create parameters for the dataset
            H5::DSetCreatPropList cparms;
            hsize_t chunks[4] = {1, 10, 10, 1};
            cparms.setChunk(4, chunks);
            cparms.setFillValue(H5::PredType::NATIVE_DOUBLE, &_noData); // set no data value,n HDF5 handles this automatically

            // create data set
            dsVals = grpAggr.createDataSet(FORECAST_DATASET_NAME, H5::PredType::NATIVE_DOUBLE, dataSpace, cparms);

            // we're creating a new file, so the baseTime given here is the start time of the dataset
            // we take the hour as a basic measure, flooring it to the hour !! this allows alternative
            // time resolutions later on...
            startTime = date::floor<std::chrono::hours>(baseTime);

            Hdf5Tools::createStringAttribute(dsVals, START_DATE_NAME, chrono::to_string(startTime));
            Hdf5Tools::createStringAttribute(dsVals, DIMENSIONS_NAME, DIMENSIONS);
            Hdf5Tools::createStringAttribute(dsVals, DESCRIPTION_NAME, DESCRIPTION);

            // --------------------------------------------------------
            // 2. create the datasets with meta data: models & stations
            // --------------------------------------------------------
            hsize_t dims2[1]    = {0};
            hsize_t maxdims2[1] = {H5S_UNLIMITED};
            H5::DataSpace dataSpace2(1, dims2, maxdims2);
            H5::DSetCreatPropList cparms2;
            hsize_t chunks2[1] = {5};
            cparms2.setChunk(1, chunks2);
            cparms2.setFillValue(_stringType, &s_noData); // set no data value

            dsModels   = grpAggr.createDataSet(MODELS_DATASET_NAME, _stringType, dataSpace2, cparms2);
            dsStations = grpAggr.createDataSet(STATION_DATASET_NAME, _stringType, dataSpace2, cparms2);
        }
        catch (const H5::Exception& e)
        {
            _logger->error("Failed to write values to HDF5 buffer: {} {}", e.getDetailMsg());
            throw RunTimeException("Failed to write values to HDF5 buffer");
        }
    }

    // -- now, store the data

    auto secsDiff      = chrono::to_seconds(baseTime - startTime);
    auto secsDiffFirst = chrono::to_seconds(forecast.firstDateTime() - baseTime);

    // get the indices
    unsigned int modelIndex   = Hdf5Tools::getIndexInStringDataSet(dsModels, _currentModel, true);
    unsigned int stationIndex = Hdf5Tools::getIndexInStringDataSet(dsStations, stationId, true);
    size_t dateIndex          = secsDiff.count() / std::chrono::seconds(_baseTimeResolution).count(); // integer division... should be ok !
    size_t fhIndex            = secsDiffFirst.count() / std::chrono::seconds(_fcTimeResolution).count();

//DEBUG !!
#ifdef DEBUG
    std::cout << "modelIndex= " << modelIndex << ", stationIndex= " << stationIndex << ", dateIndex=" << dateIndex << ", fcIndex = " << fhIndex << std::endl;
#endif


    try
    {
        // -- store data
        // a. get data space
        H5::DataSpace space = dsVals.getSpace();

        // b. get current size
        hsize_t size[4];
        space.getSimpleExtentDims(size, nullptr);
        bool extend = false;
        if (modelIndex >= size[0]) {
            size[0] = modelIndex + 1;
            extend  = true;
        }
        if (stationIndex >= size[1]) {
            size[1] = stationIndex + 1;
            extend  = true;
        }
        if (dateIndex >= size[2]) {
            size[2] = dateIndex + 1;
            extend  = true;
        }
        if (fhIndex + forecast.size() > size[3]) {
            size[3] = fhIndex + forecast.size();
            extend  = true;
        }
        // c. extend data set if required
        if (extend) {
            dsVals.extend(size);
            space.close();
            space = dsVals.getSpace();
        }
        // d. select data set hyperslab
        hsize_t count[4]  = {1, 1, 1, forecast.size()};
        hsize_t offset[4] = {modelIndex, stationIndex, dateIndex, fhIndex};
        space.selectHyperslab(H5S_SELECT_SET, count, offset);
        // e. write data to the hyperslab
        H5::DataSpace writeMemSpace(4, count);
        double fileNoData;
        dsVals.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE, &fileNoData);
        if (_noData == fileNoData) {
            // nodata placeholder in file equals the one used in the values vector
            dsVals.write(forecast.values().data(), H5::PredType::NATIVE_DOUBLE, writeMemSpace, space);
        }
        else
        {
            // nodata placeholder in file is different from the one used in the values vector
            std::vector<double> newValues(count[3]);
            for (unsigned int i = 0; i < forecast.size(); i++)
            {
                newValues[i] = forecast.value(i) == _noData ? fileNoData : forecast.value(i);
            }
            dsVals.write(newValues.data(), H5::PredType::NATIVE_DOUBLE, writeMemSpace, space);
        }
        space.close();
        dsVals.flush(H5F_SCOPE_GLOBAL);
    }
    catch (const H5::Exception& e)
    {
        throw NotAvailableException("Failed to write forecast values in forecast buffer: ({})", e.getDetailMsg());
    }
}

void Hdf5Buffer::_closeFile()
{
    if (_h5file != nullptr) {
        _h5file->close();
        delete _h5file;
    }
}

void Hdf5Buffer::_checkFullyConfigured()
{
    if (!_configured) throw RunTimeException("Hdf5Buffer Not fully configured");
}

void Hdf5Buffer::_checkIfExistsAndOpen()
{
    if (_h5file == nullptr) {
        if (!FileTools::exists(_filename))
        {
            throw RunTimeException("Cannot read from non existing file: {}", _filename);
        }

        _openFile(_filename);
    }

    return;
}

void Hdf5Buffer::_createOrOpenFile()
{

    if (_h5file == NULL) { // don't open a new file if another base time is set

        if (FileTools::exists(_filename)) {
            _openFile(_filename);
        }
        else
        {
            _createFile(_filename);
        }
    }

    return;
}

void Hdf5Buffer::_createFile(const std::string& filename)
{

    // Create a new file if not exists file
    try
    {
        _h5file = new H5::H5File(filename, H5F_ACC_TRUNC);
    }
    catch (const H5::FileIException&)
    {
        throw BadConfigurationException("Failed to create file {} (disk full? no write permissions?)", filename);
    }

    // Create OPAQ file creation tag
    try
    {
        H5::Group rootGroup = _h5file->openGroup("/");

        H5::Attribute att;
        H5::DataSpace att_space(H5S_SCALAR);

        att = rootGroup.createAttribute("HDF5BUFFER_VERSION", _stringType, att_space);
        att.write(_stringType, Hdf5BufferVersion);
        att.close();

        rootGroup.close();
    }
    catch (const H5::FileIException&)
    {
        throw BadConfigurationException("Failed to initialise OPAQ HDF5 forecast buffer...");
    }
}

void Hdf5Buffer::_openFile(const std::string& filename)
{

    // 1. open file
    try
    {
        _h5file = new H5::H5File(filename, H5F_ACC_RDWR);
    }
    catch (const H5::FileIException&)
    {
        std::stringstream ss;
        ss << "Failed to open file " << filename
           << " (is it a valid HDF5 file?)";
        throw BadConfigurationException(ss.str());
    }
}

std::chrono::hours Hdf5Buffer::getTimeResolution()
{
    return _fcTimeResolution; // return time resolution of the forecasts
}

std::chrono::hours Hdf5Buffer::getBaseTimeResolution()
{
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

TimeSeries<double> Hdf5Buffer::getValues(const chrono::date_time& t1,
                                         const chrono::date_time& t2,
                                         const std::string& stationId,
                                         const std::string& pollutantId,
                                         Aggregation::Type aggr)
{
    // return the observations stored...
    // this routine is inherited from the DataProvider...

    // what to return here ??
    throw RunTimeException("not sure what to return here, need extra information ???");

    OPAQ::TimeSeries<double> out;
    return out;
}

TimeSeries<double> Hdf5Buffer::getForecastValues(const chrono::date_time& baseTime,
                                                 const std::vector<chrono::days>& fc_hor,
                                                 const std::string& stationId,
                                                 const std::string& pollutantId,
                                                 Aggregation::Type aggr)
{
    throw RunTimeException("IMPLEMENT ME !!");

    OPAQ::TimeSeries<double> out;
    return out;
}

// return hindcast vector of model values for a fixed forecast horizon
// for a forecasted day interval
TimeSeries<double> Hdf5Buffer::getForecastValues(chrono::days fc_hor,
                                                 const chrono::date_time& fcTime1,
                                                 const chrono::date_time& fcTime2,
                                                 const std::string& stationId,
                                                 const std::string& pollutantId,
                                                 Aggregation::Type aggr)
{

    if (fcTime1 > fcTime2) throw RunTimeException("requested fcTime1 is > fcTime2...");

    H5::DataSet dsVals, dsStations, dsModels;
    H5::Group grpPol, grpAggr;
    chrono::date_time startTime;

    try
    {
        grpPol  = _h5file->openGroup(pollutantId);
        grpAggr = grpPol.openGroup(OPAQ::Aggregation::getName(aggr));
        dsVals  = grpAggr.openDataSet(FORECAST_DATASET_NAME);

        // retrieve startTime from the stored dataset in this group
        startTime = chrono::from_date_string(Hdf5Tools::readStringAttribute(dsVals, START_DATE_NAME));

        // open datasets for models & stations
        dsStations = grpAggr.openDataSet(STATION_DATASET_NAME);
        dsModels   = grpAggr.openDataSet(MODELS_DATASET_NAME);
    }
    catch (const H5::Exception&)
    {
        _logger->error("cannot retrieve hindcast values in forecast buffer...");
        throw NotAvailableException("forecast is not available");
    }

    // retrieve station, model & forecsat horizon index
    unsigned int stIndex    = Hdf5Tools::getIndexInStringDataSet(dsStations, stationId);
    unsigned int modelIndex = Hdf5Tools::getIndexInStringDataSet(dsModels, _currentModel);
    size_t fhIndex          = std::chrono::duration_cast<std::chrono::seconds>(fc_hor).count() /
                     std::chrono::duration_cast<std::chrono::seconds>(_fcTimeResolution).count();

#ifdef DEBUG
    std::cout << "Hdf5Buffer::getValues()" << std::endl;
    std::cout << "req station : " << stationId << ", idx = " << stIndex << std::endl;
    std::cout << "req model : " << _currentModel << ", idx = " << modelIndex << std::endl;
    std::cout << "req fh : " << fc_hor.count() << ", idx = " << fhIndex << std::endl;
#endif

    hsize_t modelSize = Hdf5Tools::getDataSetSize(dsVals, 0); // index 0 is models
    hsize_t stSize    = Hdf5Tools::getDataSetSize(dsVals, 1);
    //hsize_t btSize    = Hdf5Tools::getDataSetSize(dsVals, 2);
    hsize_t fhSize = Hdf5Tools::getDataSetSize(dsVals, 3);

    //
    if (fhIndex >= fhSize ||
        modelIndex >= modelSize ||
        stIndex >= stSize) {
        throw RunTimeException("Could not find requested forecast horizon/model/station in HDF5 buffer");
    }

    // compute the index for the base time of the first forecastTime
    auto b1      = fcTime1 - fc_hor;
    auto b2      = fcTime2 - fc_hor;
    size_t nvals = (chrono::to_seconds(b2 - b1).count() / getBaseTimeResolutionInSeconds().count()) + 1;

    try
    {

        OPAQ::TimeSeries<double> out;
        if (b2 < startTime) {
            // we have no values in the buffer, so return all -9999
            for (chrono::date_time fct = fcTime1; fct <= fcTime2; fct = fct + _baseTimeResolution)
                out.insert(fct, _noData);
        }
        else if (b1 < startTime)
        {
    // first forecast time before the start time...

    #ifdef DEBUG
            std::cout << "startTime : " << startTime << std::endl;
            std::cout << "forecat horizon : " << fc_hor.count() << std::endl;
            std::cout << "basetime for fcTime1 : " << b1 << std::endl;
            std::cout << "basetime for fcTime2 : " << b2 << std::endl;
    #endif

            // number of elements before the HDF5 buffer
            size_t n_before = chrono::to_seconds(startTime - b1).count() / getBaseTimeResolutionInSeconds().count();       // number of steps before buffer start
            size_t n_inside = (chrono::to_seconds(b2 - startTime).count() / getBaseTimeResolutionInSeconds().count()) + 1; // number of steps inside buffer

            if ((n_before + n_inside) != nvals) {
    #ifdef DEBUG
                std::cout << "nvals = " << nvals << std::endl;
                std::cout << "n_before = " << n_before << std::endl;
                std::cout << "n_inside = " << n_inside << std::endl;
    #endif
                throw RunTimeException("Strange things are happening... everyday... i hear the music... up above my head !");
            }

            // completely within the period
            std::vector<double> tmp(n_inside);

            // "model x station x baseTime x fcHorizon"
            H5::DataSpace space = dsVals.getSpace();
            hsize_t dc[4]       = {1, 1, n_inside, 1};
            hsize_t doffset[4]  = {modelIndex, stIndex, 0, fhIndex};
            space.selectHyperslab(H5S_SELECT_SET, dc, doffset);

            hsize_t mc[1]      = {n_inside};
            hsize_t moffset[1] = {0};
            H5::DataSpace memSpace(1, mc);
            memSpace.selectHyperslab(H5S_SELECT_SET, mc, moffset);

            dsVals.read(tmp.data(), H5::PredType::NATIVE_DOUBLE, memSpace, space);
            space.close();

            // TODO make this more efficient !!
            //      --> enable to directly dump into the timeseries object... let's worry about this later...
            auto fct = fcTime1;
            for (unsigned int ii = 0; ii < n_before; ii++)
            {
                out.insert(fct, _noData);
                fct = fct + _baseTimeResolution;
            }
            for (unsigned int ii = 0; ii < n_inside; ii++)
            {
                out.insert(fct, tmp[ii]);
                fct = fct + _baseTimeResolution;
            }
        }
        else
        {
            // completely within the period
            std::vector<double> tmp(nvals);
            size_t btIndex = chrono::to_seconds(b1 - startTime).count() / getBaseTimeResolutionInSeconds().count(); // integer division... should be ok !

            // "model x station x baseTime x fcHorizon"
            H5::DataSpace space = dsVals.getSpace();
            hsize_t dc[4]       = {1, 1, nvals, 1};
            hsize_t doffset[4]  = {modelIndex, stIndex, btIndex, fhIndex};
            space.selectHyperslab(H5S_SELECT_SET, dc, doffset);

            hsize_t mc[1]      = {nvals};
            hsize_t moffset[1] = {0};
            H5::DataSpace memSpace(1, mc);
            memSpace.selectHyperslab(H5S_SELECT_SET, mc, moffset);

#ifdef DEBUG
            std::cout << "Read values: " << fmt::format("{}x{}x{}x{}", modelIndex, stIndex, btIndex, fhIndex) << std::endl;
#endif

            dsVals.read(tmp.data(), H5::PredType::NATIVE_DOUBLE, memSpace, space);
            space.close();

            // TODO make this more efficient !!
            //      --> enable to directly dump into the timeseries object... let's worry about this later...
            auto fct = fcTime1;
            for (unsigned int ii = 0; ii < nvals; ii++)
            {
                out.insert(fct, tmp[ii]);
                fct = fct + _baseTimeResolution;
            }
        }

        return out;
    }
    catch (const H5::Exception& e)
    {
        throw NotAvailableException("Failed to retrieve forecast values in forecast buffer ({})", e.getDetailMsg());
    }
}

// return model values for a given baseTime / forecast horizon
// storage in file : "model x station x baseTime x fcHorizon"
std::vector<double> Hdf5Buffer::getModelValues(const chrono::date_time& baseTime, chrono::days fc_hor,
                                               const std::string& stationId, const std::string& pollutantId, Aggregation::Type aggr)
{

    H5::DataSet dsVals, dsStations;
    H5::Group grpPol, grpAggr;
    chrono::date_time startTime;

    try
    {
        grpPol  = _h5file->openGroup(pollutantId);
        grpAggr = grpPol.openGroup(OPAQ::Aggregation::getName(aggr));
        dsVals  = grpAggr.openDataSet(FORECAST_DATASET_NAME);

        // retrieve startTime from the stored dataset in this group
        startTime = chrono::from_date_string(Hdf5Tools::readStringAttribute(dsVals, START_DATE_NAME));

        if (startTime > baseTime)
            throw BadConfigurationException("baseTime is before start date in the dataset");

        // open datasets for models & stations
        dsStations = grpAggr.openDataSet(STATION_DATASET_NAME);
    }
    catch (const H5::Exception&)
    {
        _logger->error("cannot retrieve model values in forecast buffer...");
        throw NotAvailableException("forecast is not available");
    }

    unsigned int stIndex = Hdf5Tools::getIndexInStringDataSet(dsStations, stationId);
    size_t btIndex       = chrono::to_seconds(baseTime - startTime).count() / getBaseTimeResolutionInSeconds().count(); // integer division... should be ok !
    size_t fhIndex       = std::chrono::duration_cast<std::chrono::seconds>(fc_hor).count() /
                     std::chrono::duration_cast<std::chrono::seconds>(_fcTimeResolution).count();

#ifdef DEBUG
    std::cout << "getModelValues : stIndex= " << stIndex << ", btIndex=" << btIndex << ", fhIndex = " << fhIndex << "\n";
#endif

    unsigned int nvals = Hdf5Tools::getDataSetSize(dsVals, 0); // index 0 is models

    // initialize the output array with the number of requested values
    std::vector<double> out(nvals, getNoData());

    // now get the size of the dataset in the buffer
    hsize_t btSize = Hdf5Tools::getDataSetSize(dsVals, 2);
    hsize_t fhSize = Hdf5Tools::getDataSetSize(dsVals, 3);

    // is the station/forecast/base time in the datafile ?
    if (fhIndex < fhSize && btIndex < btSize)
    {
        // "model x station x baseTime x fcHorizon"

        H5::DataSpace space = dsVals.getSpace();
        hsize_t dc[4]       = {nvals, 1, 1, 1};
        hsize_t doffset[4]  = {0, stIndex, btIndex, fhIndex};
        space.selectHyperslab(H5S_SELECT_SET, dc, doffset);

        hsize_t mc[1]      = {nvals};
        hsize_t moffset[1] = {0};
        H5::DataSpace memSpace(1, mc);
        memSpace.selectHyperslab(H5S_SELECT_SET, mc, moffset);

        try
        {
            dsVals.read(&out[0], H5::PredType::NATIVE_DOUBLE, memSpace, space);
            space.close();
        }
        catch (const H5::DataSetIException&)
        {
            space.close();
            throw RunTimeException("Failed to read value from HDF5 buffer");
        }
    }
    else
        throw RunTimeException("Requested forecast horizon index not in HDF5 buffer");

    return out;
}

std::vector<std::string> Hdf5Buffer::getModelNames(const std::string& pollutantId, OPAQ::Aggregation::Type aggr)
{

    std::vector<std::string> out;

    H5::DataSet dsModels;
    H5::Group grpPol, grpAggr;
    // now open groups etc...
    try
    {
        grpPol  = _h5file->openGroup(pollutantId);
        grpAggr = grpPol.openGroup(OPAQ::Aggregation::getName(aggr));

        dsModels = grpAggr.openDataSet(MODELS_DATASET_NAME);
    }
    catch (const H5::Exception&)
    {
        throw NotAvailableException("Error reading model list for {}, {}", pollutantId, Aggregation::getName(aggr));
    }

    unsigned int bufferSize = Hdf5Tools::getDataSetSize(dsModels);
    std::vector<char*> buffer(bufferSize);
    Hdf5Tools::readStringData(buffer.data(), dsModels);
    for (unsigned int i = 0; i < bufferSize; i++)
    {
        out.push_back(buffer[i]);
        free(buffer[i]);
    }

    dsModels.close();
    grpAggr.close();
    grpPol.close();

    return out;
}

std::chrono::seconds Hdf5Buffer::getBaseTimeResolutionInSeconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(getBaseTimeResolution());
}

}
