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

class StringTools {
public:
	StringTools();
	virtual ~StringTools();

	static bool replace(std::string& str, const std::string& from, const std::string& to);
	static void replaceAll(std::string& str, const std::string& from, const std::string& to);
	static std::vector<std::string> tokenize(const std::string &source,
			const char *delimiter = " \t\n\r\f", const int delimiterCount = 5, bool keepEmpty = false);
	static int find (char * list [], unsigned int listSize, const std::string & item);

private:
	static size_t findFirstDelimiter (const std::string& str, const char *delimiter, const int delimiterCount, size_t prev);
};

} /* namespace test */
#endif /* STRINGTOOLS_H_ */
