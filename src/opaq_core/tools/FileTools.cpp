/*
 * FileTools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "FileTools.h"

namespace OPAQ {

FileTools::FileTools() {}

FileTools::~FileTools() {}

bool FileTools::exists (const std::string & filename) {
	/*
	 * see https://stackoverflow.com/a/12774387
	 */
	if (FILE *file = fopen(filename.c_str(), "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

bool FileTools::del (const std::string & filename) {
	if (exists(filename)) {
		return remove(filename.c_str()) == 0;
	} else {
		return true;
	}

}

} /* namespace OPAQ */
