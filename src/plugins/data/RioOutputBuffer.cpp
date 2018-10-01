#include "RioOutputBuffer.h"
#include "DateTime.h"
#include "Grid.h"
#include "Hdf5Tools.h"
#include "Pollutant.h"
#include "Station.h"
#include "infra/configdocument.h"
#include "infra/log.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>

namespace opaq {

using namespace inf;
using namespace chrono_literals;

static const std::string s_gridGroupName("grid");
static const std::string s_stationsGroupName("stations");
static const std::string s_timeGroupName("time");

// set version information for this H5 store plugin
static const std::string s_rioBufferVersion("1.1");

static std::string s_pollutantPlaceholder   = "%pol%";
static std::string s_aggregationPlaceholder = "%aggr%";
static std::string s_gridPlaceholder        = "%grid%";
static std::string s_gisPlaceholder         = "%gis%";
static std::string s_startDatePlaceholder   = "%startdate%";
static std::string s_endDatePlaceholder     = "%enddate%";

static void writeGrid(chrono::date_time start, chrono::date_time end, const Grid& grid, H5::Group& group)
{
    hsize_t dimension = grid.cellCount();
    H5::DataSpace dataSpace(1, &dimension);

    auto xDataSet = group.createDataSet("x", H5::PredType::NATIVE_DOUBLE, dataSpace);
    auto yDataSet = group.createDataSet("y", H5::PredType::NATIVE_DOUBLE, dataSpace);

    hsize_t valuesDimension[2]{grid.cellCount(), hsize_t(date::floor<chrono::days>(end - start).count())};
    auto valueDataSet = group.createDataSet("value", H5::PredType::NATIVE_DOUBLE, H5::DataSpace(2, valuesDimension));

    H5::StrType stringType(0, H5T_VARIABLE);
    hsize_t attrDimension = 1;
    H5::DataSpace attrSpace(1, &attrDimension);
    valueDataSet.createAttribute("description", stringType, attrSpace);
    valueDataSet.createAttribute("units", stringType, attrSpace);
    valueDataSet.createAttribute("scale_factor", stringType, attrSpace);
    valueDataSet.createAttribute("missing_value", stringType, attrSpace);

    std::vector<double> xValues, yValues;
    xValues.reserve(dimension);
    yValues.reserve(dimension);
    std::transform(grid.begin(), grid.end(), std::back_inserter(xValues), [](const Cell& cell) { return cell.getXc(); });
    std::transform(grid.begin(), grid.end(), std::back_inserter(yValues), [](const Cell& cell) { return cell.getYc(); });

    xDataSet.write(xValues.data(), H5::PredType::NATIVE_DOUBLE);
    yDataSet.write(yValues.data(), H5::PredType::NATIVE_DOUBLE);
}

static void writeStations(const std::vector<Station>& stations, H5::Group& group)
{
    hsize_t dimension = stations.size();
    H5::DataSpace dataSpace(1, &dimension);

    H5::StrType stringType(0, H5T_VARIABLE);
    auto namesSet = group.createDataSet("name", stringType, dataSpace);
    auto xDataSet = group.createDataSet("x", H5::PredType::NATIVE_DOUBLE, dataSpace);
    auto yDataSet = group.createDataSet("y", H5::PredType::NATIVE_DOUBLE, dataSpace);

    std::vector<const char*> names;
    std::vector<double> xValues, yValues;
    names.reserve(stations.size());
    names.reserve(xValues.size());
    names.reserve(yValues.size());

    std::transform(stations.begin(), stations.end(), std::back_inserter(names), [](const Station& station) { return station.getName().c_str(); });
    std::transform(stations.begin(), stations.end(), std::back_inserter(xValues), [](const Station& station) { return station.getX(); });
    std::transform(stations.begin(), stations.end(), std::back_inserter(yValues), [](const Station& station) { return station.getY(); });

    namesSet.write(names.data(), stringType);
    xDataSet.write(xValues.data(), H5::PredType::NATIVE_DOUBLE);
    yDataSet.write(yValues.data(), H5::PredType::NATIVE_DOUBLE);
}

RioOutputBuffer::RioOutputBuffer()
: _index(0)
, _stringType(H5::StrType(0, H5T_VARIABLE))
{
    // Tell the hdf5 lib not to print error messages: we will handle them properly ourselves
    H5::Exception::dontPrint();
}

std::string RioOutputBuffer::name()
{
    return "riooutputbuffer";
}

void RioOutputBuffer::configure(const ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    if (!configuration) {
        throw NullPointerException("No configuration element given for RioOutputBuffer...");
    }

    _filePattern = std::string(configuration.child("filename").value());
}

void RioOutputBuffer::openResultsFile(chrono::date_time start, chrono::date_time end,
    const Pollutant& pol, Aggregation::Type agg,
    const std::vector<Station>& stations, const Grid& grid, GridType gridType)
{
    throwIfNotConfigured();

    _index = 0;

    auto filename = _filePattern;
    boost::algorithm::replace_all(filename, s_pollutantPlaceholder, pol.getName());
    boost::algorithm::replace_all(filename, s_aggregationPlaceholder, Aggregation::getName(agg));
    boost::algorithm::replace_all(filename, s_gisPlaceholder, "GIS");
    boost::algorithm::replace_all(filename, s_gridPlaceholder, gridTypeToString(gridType));
    boost::algorithm::replace_all(filename, s_startDatePlaceholder, chrono::to_dense_date_string(start));
    boost::algorithm::replace_all(filename, s_endDatePlaceholder, chrono::to_dense_date_string(end));

    try {
        _h5File = std::make_unique<H5::H5File>(filename, H5F_ACC_TRUNC);

        auto rootGroup = _h5File->openGroup("/");

        auto att = rootGroup.createAttribute("version", _stringType, H5::DataSpace(H5S_SCALAR));
        att.write(_stringType, s_rioBufferVersion);

        auto timeGroup = rootGroup.createGroup(s_timeGroupName);

        auto gridGroup = rootGroup.createGroup(s_gridGroupName);
        writeGrid(start, end, grid, gridGroup);

        auto stationsGroup = rootGroup.createGroup(s_stationsGroupName);
        writeStations(stations, stationsGroup);
    } catch (const H5::FileIException& e) {
        throw RuntimeError("Failed to create file {} ({}::{})", filename, e.getCFuncName(), e.getCDetailMsg());
    } catch (const H5::Exception& e) {
        throw RuntimeError("Failed to create Rio output buffer ({}::{})", e.getCFuncName(), e.getCDetailMsg());
    }
}

void RioOutputBuffer::addResults(const std::vector<double>& results)
{
    assert(_h5File);

    try {
        auto gridGroup = _h5File->openGroup("/" + s_gridGroupName);
        auto ds        = gridGroup.openDataSet("value");

        hsize_t size[4]{0};
        auto space = ds.getSpace();
        space.getSimpleExtentDims(size);

        assert(_index <= size[1]);

        hsize_t count[4]  = {size[0], 1, 1, 1};
        hsize_t offset[4] = {0, _index, 0, 0};
        space.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace writeMemSpace(4, count);
        ds.write(results.data(), H5::PredType::NATIVE_DOUBLE, writeMemSpace, space);

        ++_index;
    } catch (const H5::Exception& e) {
        throw RuntimeError("Failed to add results to Rio output buffer ({}::{})", e.getCFuncName(), e.getCDetailMsg());
    }
}

void RioOutputBuffer::closeResultsFile()
{
    _h5File.reset();
}

void RioOutputBuffer::throwIfNotConfigured() const
{
    if (_filePattern.empty()) {
        throw RuntimeError("Hdf5Buffer Not fully configured");
    }
}

}
