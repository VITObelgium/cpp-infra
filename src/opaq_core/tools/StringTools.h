/*
 * StringTools.h
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>

namespace opaq {
namespace StringTools {

std::vector<std::string> tokenize(std::string_view source, const char* delimiter = " \t\n\r\f", const int delimiterCount = 5, bool keepEmpty = false);
int find(char* list[], unsigned int listSize, const std::string& item);
}
}
