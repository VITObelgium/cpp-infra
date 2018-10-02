#include <iostream>

#include "h5writer.hpp"
#include "parser.hpp"

/*
TODO
- sanitize how the cnf, net and grid are passed to this object... a bit funky now... 

  std::cout << "TODO (h5writer::init) : " << std::endl
            << "  - create and store memory dataspaces, what to store from constructor ? "
            << "  - get missing_value from somewhere (config)" << std::endl
            << "  - get scale factor from somewhere... " << std::endl
            << "  - get compression from somewhere" << std::endl
            << "  - calculate the dataset size beforehand from the given dates ??" << std::endl;
*/

namespace rio {

using namespace inf;

h5writer::h5writer(const XmlNode& cnf)
: outputhandler(cnf)
, _h5FileName("rio_output.h5")
, _h5File(nullptr)
, _data_scale_factor(1.0)
, _data_fill_value(-9999.)
, _time_fill_value(0.)
, _detection_limit(1.0)
, _compression(9)
, _write_hours(false)
, _write_count(0)
{
    try {
        _h5FileName = _xml.get<std::string>("handler.location");
    } catch (...) {
        throw std::runtime_error("invalid configuration in h5writer XML config, <location> required");
    }
}

h5writer::~h5writer()
{
}

void h5writer::init(const rio::config& cnf,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    _net  = net;
    _grid = grid;

    if (!cnf.aggr().compare("1h"))
        _write_hours = true;

    // tell hdf5 lib not to print err messages, we will do the error handling
    H5::Exception::dontPrint();
    rio::parser::get()->process(_h5FileName);

    try {
        std::cout << " Opening " << _h5FileName << std::endl;
        // I don't understand why this craps out if i try to create a normal variable instead of a pointer
        // hmmm
        _h5File = std::make_unique<H5::H5File>(_h5FileName, H5F_ACC_TRUNC);
    } catch (const H5::Exception& err) {
        throw std::runtime_error("unable to open " + _h5FileName + " : " + err.getDetailMsg());
    }

    // generate the structure
    _createGroups();
    _createDataSets(cnf);
    _setAttributes(cnf);
    _initStationInfo(net);
    _initGridInfo(grid);

    return;
}

void h5writer::_createGroups(void)
{
    try {
        _stGrp = _h5File->createGroup("stations");
        _grGrp = _h5File->createGroup("grid");
        _tmGrp = _h5File->createGroup("time");
    } catch (const H5::Exception& err) {
        throw std::runtime_error("unable to create HDF5 groups : " + err.getDetailMsg());
    }

    return;
}

void h5writer::_createDataSets(const rio::config& cnf)
{
    try {
        hsize_t dims[2]    = {_net->size(), 1};
        hsize_t maxdims[2] = {_net->size(), H5S_UNLIMITED};
        _st_dspace.setExtentSimple(2, dims, maxdims);

        H5::DSetCreatPropList cparms_stations;
        cparms_stations.setChunk(2, dims);
        cparms_stations.setFillValue(H5::PredType::NATIVE_FLOAT, &_data_fill_value);
        cparms_stations.setDeflate(_compression);

        _stDS = _stGrp.createDataSet("value", H5::PredType::NATIVE_FLOAT, _st_dspace, cparms_stations);
        seth5attr(_stDS, "scale_factor", _data_scale_factor);
        seth5attr(_stDS, "missing_value", _data_fill_value);
        seth5attr(_stDS, "description", "observed concentration value");
        seth5attr(_stDS, "units", "ug/m3");

        // 3. Create the grids dataset
        dims[0]    = _grid->size();
        maxdims[0] = _grid->size();
        _gr_dspace.setExtentSimple(2, dims, maxdims);
        H5::DSetCreatPropList cparms_grid;
        cparms_grid.setChunk(2, dims);
        cparms_grid.setFillValue(H5::PredType::NATIVE_FLOAT, &_data_fill_value);
        cparms_grid.setDeflate(_compression);

        _grValDS = _grGrp.createDataSet("value", H5::PredType::NATIVE_FLOAT, _gr_dspace, cparms_grid);
        seth5attr(_grValDS, "scale_factor", _data_scale_factor);
        seth5attr(_grValDS, "missing_value", _data_fill_value);
        seth5attr(_grValDS, "description", "gridded concentration value");
        seth5attr(_grValDS, "units", "ug/m3");

        _grErrDS = _grGrp.createDataSet("error", H5::PredType::NATIVE_FLOAT, _gr_dspace, cparms_grid);
        seth5attr(_grErrDS, "scale_factor", _data_scale_factor);
        seth5attr(_grErrDS, "missing_value", _data_fill_value);
        seth5attr(_grErrDS, "description", "error on gridded concentration value");
        seth5attr(_grErrDS, "units", "ug/m3");

        //4. Create the time arrays
        hsize_t tm_dims[1]    = {1};
        hsize_t tm_maxdims[1] = {H5S_UNLIMITED};
        _tm_dspace.setExtentSimple(1, tm_dims, tm_maxdims);
        H5::DSetCreatPropList cparms_time;
        cparms_time.setChunk(1, tm_dims);
        cparms_time.setFillValue(H5::PredType::NATIVE_INT, &_time_fill_value);
        cparms_time.setDeflate(_compression);

        _yrDS = _tmGrp.createDataSet("year", H5::PredType::NATIVE_INT, _tm_dspace, cparms_time);
        _mnDS = _tmGrp.createDataSet("month", H5::PredType::NATIVE_INT, _tm_dspace, cparms_time);
        _dyDS = _tmGrp.createDataSet("day", H5::PredType::NATIVE_INT, _tm_dspace, cparms_time);
        _hrDS;
        if (!cnf.aggr().compare("1h"))
            _hrDS = _tmGrp.createDataSet("hour", H5::PredType::NATIVE_INT, _tm_dspace, cparms_time);
    } catch (const H5::Exception& err) {
        throw std::runtime_error("unable to create HDF5 datasets : " + err.getDetailMsg());
    }
    return;
}

void h5writer::_setAttributes(const rio::config& cnf)
{
    // h5File is stored as unique ptr, so need to dereference it
    seth5attr(*_h5File, "pollutant", cnf.pol());
    seth5attr(*_h5File, "agg_time", cnf.aggr());
    seth5attr(*_h5File, "grid_type", cnf.grid());
    seth5attr(*_h5File, "gis_type", cnf.ipol());
    seth5attr(*_h5File, "ipol_mode", cnf.ipol_class());

    seth5attr(*_h5File, "config_version", cnf.configuration());
    seth5attr(*_h5File, "code_implementation", "c++");
    seth5attr(*_h5File, "code_version", "5.0"); // TODO : put in version number here
    seth5attr(*_h5File, "output_format", "1.2");

    seth5attr(*_h5File, "missing_value", _data_fill_value);
    seth5attr(*_h5File, "scale_factor", _data_scale_factor);
    seth5attr(*_h5File, "detection_limit", _detection_limit);

    return;
}

void h5writer::_initStationInfo(const std::shared_ptr<rio::network> n)
{
    std::vector<double> x, y, alt;
    std::vector<int> type;
    std::vector<std::string> st_name, wkt;
    for (const rio::station& s : n->st_list()) {
        x.push_back(s.x());
        y.push_back(s.y());
        alt.push_back(s.alt());
        type.push_back(s.type());
        st_name.push_back(s.name());
        wkt.push_back(s.wkt());
    }

    writeVector(_stGrp, "name", st_name);
    writeVector(_stGrp, "x", x);
    writeVector(_stGrp, "y", y);
    writeVector(_stGrp, "wkt", wkt);
    writeVector(_stGrp, "alt", alt);
    writeVector(_stGrp, "type", type);

    return;
}

void h5writer::_initGridInfo(const std::shared_ptr<rio::grid> g)
{
    std::vector<double> cx, cy, alt;
    std::vector<std::string> wkt;
    for (const rio::cell& c : g->cells()) {
        cx.push_back(c.cx());
        cy.push_back(c.cy());
        alt.push_back(c.alt());
        wkt.push_back(c.wkt());
    }

    writeVector(_grGrp, "x", cx);
    writeVector(_grGrp, "y", cy);
    writeVector(_grGrp, "alt", alt);
    writeVector(_grGrp, "wkt", wkt);

    return;
}

void h5writer::writeVector(H5::H5Location& grp, const char* name, const std::vector<double>& x)
{
    hsize_t dims[1] = {x.size()};
    H5::DataSpace data_space(1, dims);
    H5::DataSet ds = grp.createDataSet(name, H5::PredType::NATIVE_DOUBLE, data_space);
    ds.write(x.data(), H5::PredType::NATIVE_DOUBLE);

    return;
}

void h5writer::writeVector(H5::H5Location& grp, const char* name, const std::vector<float>& x)
{
    hsize_t dims[1] = {x.size()};
    H5::DataSpace data_space(1, dims);
    H5::DataSet ds = grp.createDataSet(name, H5::PredType::NATIVE_FLOAT, data_space);
    ds.write(x.data(), H5::PredType::NATIVE_FLOAT);

    return;
}

void h5writer::writeVector(H5::H5Location& grp, const char* name, const std::vector<int>& x)
{
    hsize_t dims[1] = {x.size()};
    H5::DataSpace data_space(1, dims);
    H5::DataSet ds = grp.createDataSet(name, H5::PredType::NATIVE_INT, data_space);
    ds.write(x.data(), H5::PredType::NATIVE_INT);

    return;
}

/*
 NOTE/ this actually dumps the fwhole verctor in a single dataspace, which is not really the idea
       but could be usefull for attributes

void h5writer::writeVector( H5::H5Location &grp, const char *name, const std::vector<std::string>& str )
{  
  // create vector of const char* for HDF
  std::vector<const char*> data;
  for ( const auto& s : str ) data.push_back( s.data() );
  
  // create a buffer 
  hvl_t buffer;
  buffer.p = data.data();
  buffer.len = data.size();

  // construct the datatype for a vector of strings
  H5::StrType str_type( H5::PredType::C_S1, H5T_VARIABLE );
  str_type.setCset( H5T_CSET_UTF8 );
  auto svec_type = H5::VarLenType( &str_type );

  // and dump !
  H5::DataSet ds = grp.createDataSet( name, svec_type, H5S_SCALAR );
  ds.write( &hdf_buffer, svec_type );

  return;
}
*/

void h5writer::writeVector(H5::H5Location& grp, const char* name, const std::vector<std::string>& str)
{
    // construct the datatype for a vector of strings
    H5::StrType str_type(H5::PredType::C_S1, H5T_VARIABLE);
    str_type.setCset(H5T_CSET_UTF8);

    hsize_t dims[] = {str.size()};
    H5::DataSpace data_space(1, dims);
    dims[0] = 1;
    H5::DataSpace mem_space(1, dims);

    hsize_t count[]  = {1};
    hsize_t offset[] = {0};
    mem_space.selectHyperslab(H5S_SELECT_SET, count, offset);

    H5::DataSet ds = grp.createDataSet(name, str_type, data_space);
    for (const std::string& s : str) {
        const char* ss = s.c_str();
        data_space.selectHyperslab(H5S_SELECT_SET, count, offset);
        ds.write(&ss, str_type, mem_space, data_space);
        offset[0] += 1;
    }

    return;
}

void h5writer::seth5attr(H5::H5Object& l, const char* name, double value)
{
    hsize_t dims[1] = {1};
    H5::Attribute att =
        l.createAttribute(name, H5::PredType::NATIVE_DOUBLE, H5::DataSpace(1, dims));
    att.write(H5::PredType::NATIVE_DOUBLE, &value);

    return;
}

void h5writer::seth5attr(H5::H5Object& l, const char* name, float value)
{
    hsize_t dims[1] = {1};
    H5::Attribute att =
        l.createAttribute(name, H5::PredType::NATIVE_FLOAT, H5::DataSpace(1, dims));
    att.write(H5::PredType::NATIVE_FLOAT, &value);

    return;
}

void h5writer::seth5attr(H5::H5Object& l, const char* name, int value)
{
    hsize_t dims[1] = {1};
    H5::Attribute att =
        l.createAttribute(name, H5::PredType::NATIVE_INT, H5::DataSpace(1, dims));
    att.write(H5::PredType::NATIVE_INT, &value);

    return;
}

void h5writer::seth5attr(H5::H5Object& l, const char* name, std::string value)
{
    hsize_t dims[1] = {1};
    H5::StrType str_type(H5::PredType::C_S1, value.size());
    str_type.setSize(value.size());
    H5::DataSpace att_space = H5::DataSpace(1, dims);
    H5::Attribute att       = l.createAttribute(name, str_type, att_space);
    att.write(str_type, value.data());

    return;
}

/**
 * This is where the main data writing happens...
 * */
void h5writer::write(const boost::posix_time::ptime& curr_time,
    const std::map<std::string, double>& obs,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{
    // 1. fill observations in array of whole network
    // set default to missing value
    std::vector<double> st_data(_net->size(), _data_fill_value);
    for (unsigned int i = 0; i < _net->size(); i++) {
        auto it = obs.find(_net->st_list().at(i).name());
        if (it != obs.end()) st_data[i] = it->second;
    }

    // 2. write the station data
    writeField(_stDS, st_data.data());

    // 3. write the gridded data
    writeField(_grValDS, values.data());
    writeField(_grErrDS, uncert.data());

    // 4. write the timestamp
    int year  = curr_time.date().year();
    int month = curr_time.date().month();
    int day   = curr_time.date().day();

    writeField(_yrDS, &year);
    writeField(_mnDS, &month);
    writeField(_dyDS, &day);

    if (_write_hours) {
        int hour = curr_time.time_of_day().hours();
        writeField(_hrDS, &hour);
    }

    // increment the write counter
    _write_count++;

    return;
}

template <typename data_type>
void h5writer::writeField(H5::DataSet& ds, const data_type* vals)
{
    // get the first dimension size from the dataspace itself, the second we keep
    // internally as counter to avoid trouble with the first write
    const int n = ds.getSpace().getSimpleExtentNdims();
    std::vector<hsize_t> ds_dims(n);
    ds.getSpace().getSimpleExtentDims(ds_dims.data());
    ds_dims[n - 1] = _write_count + 1; // last dimension is the one we increment
    ds.extend(ds_dims.data());
    H5::DataSpace file_space = ds.getSpace(); // need to re-get the space

    // construct offset and count, act on last dimension
    std::vector<hsize_t> count(n);
    std::vector<hsize_t> offset(n);
    for (int i = 0; i < n; i++) {
        count[i]  = ds_dims[i];
        offset[i] = 0;
    }
    count[n - 1]  = 1;
    offset[n - 1] = _write_count;

    // the memory dataspace is either the length of the array or 1 in case of the
    // time arrays --> if n is 1
    hsize_t dims[1];
    if (n == 2)
        dims[0] = ds_dims[0];
    else
        dims[0] = 1;
    H5::DataSpace mem_space(1, dims);

    H5::DataType hdf5_type;
    if (std::is_same<data_type, double>::value) {
        hdf5_type = H5::PredType::NATIVE_DOUBLE;
    } else if (std::is_same<data_type, int>::value) {
        hdf5_type = H5::PredType::NATIVE_INT;
    } else {
        throw std::runtime_error("writeField only for int or double");
    }

    // debug
    /*
  for ( int i = 0; i < n; i ++ ) std::cout
    << "count[" << i << "]=" << count[i] 
    << ", offset[" << i << "]=" << offset[i]
    << ", mem dim : " << dims[0] << std::endl;
  */

    file_space.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());
    ds.write(vals, hdf5_type, mem_space, file_space);

    return;
}

void h5writer::close(void)
{
    std::cout << "Closing hdf5 output in " << _h5FileName << "..." << std::endl;
    _h5File->close();

    return;
}

} // namespace rio
