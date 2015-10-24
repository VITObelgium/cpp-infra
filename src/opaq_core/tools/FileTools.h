/*
 * FileTools.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef FILETOOLS_H_
#define FILETOOLS_H_

#include <string>
#include <stdio.h>

namespace OPAQ {

class FileTools {
public:
	FileTools();
	virtual ~FileTools();

	static bool exists (const std::string & filename);
	static bool del (const std::string & filename);
};

} /* namespace OPAQ */
#endif /* FILETOOLS_H_ */
