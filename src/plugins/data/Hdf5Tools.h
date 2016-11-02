/*
 * Hdf5Tools.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef HDF5TOOLS_H_
#define HDF5TOOLS_H_

#include <H5Cpp.h>
#include <cstdlib>
#include <vector>

namespace opaq {

class Hdf5Tools {
public:
	Hdf5Tools();
	virtual ~Hdf5Tools();

	//static const H5::StrType stringType;

	static unsigned int getDataSetSize (const H5::DataSet & dataSet, const unsigned int dimIndex = 0);
	static void createStringAttribute(const H5::DataSet & dataSet, const std::string & attname,
			const std::string & attvalue);
	static std::string readStringAttribute (const H5::DataSet & dataSet, const std::string & name);
	/*
	 * @param craete if set to true: if the string is not found in the data set, it is added to it and its index is returned.
	 */
	static int getIndexInStringDataSet (H5::DataSet & dataSet, const std::string &string, bool create = false);
	static std::vector<std::string> readStringData(const H5::DataSet & dataSet);
	static void readLongData (long * buffer, const H5::DataSet & dataSet);
	static void addToStringDataSet (H5::DataSet & dataSet, const std::string & value);
	static void addToLongDataSet (H5::DataSet & dataSet, const long & value);

};

} /* namespace OPAQ */
#endif /* HDF5TOOLS_H_ */
