#pragma once

#include <H5Cpp.h>
#include <cstdlib>
#include <vector>

namespace opaq
{
namespace Hdf5Tools
{

unsigned int getDataSetSize(const H5::DataSet& dataSet, const unsigned int dimIndex = 0);
void createStringAttribute(const H5::DataSet& dataSet, const std::string& attname, const std::string& attvalue);
std::string readStringAttribute(const H5::DataSet& dataSet, const std::string& name);

/*
    * @param craete if set to true: if the string is not found in the data set, it is added to it and its index is returned.
    */
int getIndexInStringDataSet(H5::DataSet& dataSet, const std::string& string, bool create = false);
std::vector<std::string> readStringData(const H5::DataSet& dataSet);
void readLongData(long* buffer, const H5::DataSet& dataSet);
void addToStringDataSet(H5::DataSet& dataSet, const std::string& value);
void addToLongDataSet(H5::DataSet& dataSet, const long& value);

}
}
