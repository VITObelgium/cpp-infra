/*
 * GzipReader.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "GzipReader.h"

namespace OPAQ {

GzipReader::GzipReader() {
	_igzstream = NULL;
}

GzipReader::~GzipReader() {
	close();
}

void GzipReader::open(const std::string & filename) {
	close();
	if (! FileTools::exists(filename)) {
		std::stringstream ss;
		ss << "File not found: " << filename;
		throw IOException(ss.str());
	}
	_igzstream = new igzstream(filename.c_str());
	if (! _igzstream->good()) {
		std::stringstream ss;
		ss << "Failed to open file " << filename;
		throw IOException(ss.str());
	}
}

std::string GzipReader::readLine() {
	std::string line;
	if (_igzstream != NULL)
		std::getline(*_igzstream, line);
	return line;
}

bool GzipReader::eof() {
	if (_igzstream != NULL) {
		return _igzstream->eof();
	} else {
		return true;
	}
}

void GzipReader::close() {
	if (_igzstream != NULL) {
		_igzstream->close();
		delete _igzstream;
		_igzstream = NULL;
	}
}


} /* namespace OPAQ */
