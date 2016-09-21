/*
 * Hdf5Tools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Hdf5Tools.h"

namespace OPAQ {

Hdf5Tools::Hdf5Tools() {}

Hdf5Tools::~Hdf5Tools() {}

//const H5::StrType Hdf5Tools::stringType = H5::StrType(0, H5T_VARIABLE);

unsigned int Hdf5Tools::getDataSetSize (const H5::DataSet & dataSet, const unsigned int dimIndex) {
	H5::DataSpace space = dataSet.getSpace();
	unsigned int rank = space.getSimpleExtentNdims();
	std::vector<hsize_t> size(rank);
	space.getSimpleExtentDims(size.data(), nullptr);
	space.close();
	return static_cast<unsigned int>(size[dimIndex]);
}

void Hdf5Tools::createStringAttribute(const H5::DataSet & dataSet, const std::string & attname,
	const std::string & attvalue) {

    static H5::StrType stringType = H5::StrType(0, H5T_VARIABLE);

	H5::DataSpace att_space(H5S_SCALAR);
	H5::Attribute att = dataSet.createAttribute(attname, stringType, att_space);
	att.write(stringType, attvalue);
}

std::string Hdf5Tools::readStringAttribute (const H5::DataSet & dataSet, const std::string & name) {
	std::string out;
    static H5::StrType stringType = H5::StrType(0, H5T_VARIABLE);
    
    H5::Attribute att = dataSet.openAttribute(name);
	att.read(stringType, out);
	att.close();
	return out;
}

int Hdf5Tools::getIndexInStringDataSet (H5::DataSet & dataSet, const std::string &string, bool create) {
	// 1. read parameter data set
	unsigned int bufferSize = getDataSetSize(dataSet);
	std::vector<char*> buffer(bufferSize);
	readStringData(buffer.data(), dataSet);

	// 2. check if given parameter is already in the set
	int index = StringTools::find(buffer.data(), bufferSize, string);
	if (index < 0 && create) {
		// if not found; add to list and update index value
		addToStringDataSet(dataSet, string);
		index = bufferSize;
	}

	// 3. clean up buffer;
	for (unsigned int i = 0; i < bufferSize; i++) {
		free(buffer[i]);
	}

	// 4. return parameter index
	return index;

}

void Hdf5Tools::readStringData (char ** buffer, const H5::DataSet & dataSet) {
	// get data set data space
	H5::DataSpace space = dataSet.getSpace();
	// get data size
	hsize_t size[1];
	space.getSimpleExtentDims(size, NULL);
	// create data set hyperslab
	hsize_t count[1]; count[0] = size[0];
	hsize_t offset[1]; offset[0] = 0;
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// define memory data space
	H5::DataSpace memSpace(1, size);
	// define memory hyperslab
	memSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
	// read data
    static H5::StrType stringType = H5::StrType(0, H5T_VARIABLE);
	dataSet.read(buffer, stringType, memSpace, space);
	space.close();
	memSpace.close();
}

void Hdf5Tools::readLongData (long * buffer, const H5::DataSet & dataSet) {
	// get data set data space
	H5::DataSpace space = dataSet.getSpace();
	// get data size
	hsize_t size[1];
	space.getSimpleExtentDims(size, NULL);
	// create data set hyperslab
	hsize_t count[1]; count[0] = size[0];
	hsize_t offset[1]; offset[0] = 0;
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// define memory data space
	H5::DataSpace memSpace(1, size);
	// define memory hyperslab
	memSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
	// read data
	dataSet.read(buffer, H5::PredType::NATIVE_LONG, memSpace, space);
	space.close();
	memSpace.close();
}

void Hdf5Tools::addToStringDataSet (H5::DataSet & dataSet, const std::string & value) {
	// get data set data space
	H5::DataSpace space = dataSet.getSpace();
	// get data size
	hsize_t size[1];
	space.getSimpleExtentDims(size, NULL);
	// extend the data set
	size[0]++;
	dataSet.extend(size);
	space.close();
	// select data hyperslab
	space = dataSet.getSpace();
	hsize_t count[1]; count[0] = 1;
	hsize_t offset[1]; offset [0] = size[0] - 1;
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// write data to the hyperslab
	std::string writeBuffer [1]; writeBuffer[0] = value;
	hsize_t writeSize [1]; writeSize[0] = 1;
	H5::DataSpace writeMemSpace (1, writeSize);
    static H5::StrType stringType = H5::StrType(0, H5T_VARIABLE);
    dataSet.write(writeBuffer, stringType, writeMemSpace, space);
	space.close();
	dataSet.flush(H5F_SCOPE_GLOBAL);
}



void Hdf5Tools::addToLongDataSet (H5::DataSet & dataSet, const long & value) {
	// get data set data space
	H5::DataSpace space = dataSet.getSpace();
	// get data size
	hsize_t size[1];
	space.getSimpleExtentDims(size, NULL);
	// extend the data set
	size[0]++;
	dataSet.extend(size);
	space.close();
	// select data hyperslab
	space = dataSet.getSpace();
	hsize_t count[1]; count[0] = 1;
	hsize_t offset[1]; offset [0] = size[0] - 1;
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// write data to the hyperslab
	hsize_t writeSize [1]; writeSize[0] = 1;
	H5::DataSpace writeMemSpace (1, writeSize);
	dataSet.write(&value, H5::PredType::NATIVE_LONG, writeMemSpace, space);
	space.close();
	dataSet.flush(H5F_SCOPE_GLOBAL);
}

} /* namespace OPAQ */
