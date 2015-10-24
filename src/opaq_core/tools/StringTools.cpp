/*
 * StringTools.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#include "StringTools.h"

namespace OPAQ {

StringTools::StringTools() {}

StringTools::~StringTools() {}

bool StringTools::replace(std::string& str, const std::string& from,
		const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

void StringTools::replaceAll(std::string& str, const std::string& from,
		const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

std::vector<std::string> StringTools::tokenize(const std::string &source,
		const char *delimiter, const int delimiterCount, bool keepEmpty) {
	std::vector<std::string> results;

	size_t prev = 0;
	size_t next = 0;

	while ((next = findFirstDelimiter(source, delimiter, delimiterCount, prev)) != std::string::npos) {
		if (keepEmpty || (next - prev != 0)) {
			results.push_back(source.substr(prev, next - prev));
		}
		prev = next + 1;
	}

	if (prev < source.size()) {
		results.push_back(source.substr(prev));
	}

	return results;
}

size_t StringTools::findFirstDelimiter (const std::string& str, const char *delimiter, const int delimiterCount, size_t prev) {
	size_t out = std::string::npos;
	for (int i = 0; i < delimiterCount; i++) {
		size_t pos = str.find_first_of(delimiter[i], prev);
		if (pos < out) out = pos;
	}
	return out;
}

int StringTools::find (char * list [], unsigned int listSize, const std::string & item) {
	for (unsigned int i = 0; i < listSize; i++) {
		if (std::string(list[i]).compare(item) == 0)
			return i;
	}
	return -1;	// not found
}


char* StringTools::trim( char *str ) {

  char *end;
  // Trim leading space
  while(isspace(*str)) str++;
  if(*str == 0) return str;
  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  // Write new null terminator
  *(end+1) = 0;
  
  return str;
}


} /* namespace test */
