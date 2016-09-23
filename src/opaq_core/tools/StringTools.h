/*
 * StringTools.h
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#ifndef STRINGTOOLS_H_
#define STRINGTOOLS_H_

#include <string>
#include <vector>

namespace OPAQ {

namespace StringTools {
    bool replace(std::string& str, const std::string& from, const std::string& to);
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::vector<std::string> tokenize(const std::string &source,
			const char *delimiter = " \t\n\r\f", const int delimiterCount = 5, bool keepEmpty = false);
    int find (char * list [], unsigned int listSize, const std::string & item);
}

} /* namespace test */
#endif /* STRINGTOOLS_H_ */
