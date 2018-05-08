/*
 * StringTools.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#include "StringTools.h"

namespace opaq {
namespace StringTools {

static size_t findFirstDelimiter(std::string_view str, const char* delimiter, const int delimiterCount, size_t prev)
{
    size_t out = std::string::npos;
    for (int i = 0; i < delimiterCount; i++) {
        size_t pos = str.find_first_of(delimiter[i], prev);
        if (pos < out) out = pos;
    }
    return out;
}

std::vector<std::string> tokenize(std::string_view source, const char* delimiter, const int delimiterCount, bool keepEmpty)
{
    std::vector<std::string> results;

    size_t prev = 0;
    size_t next = 0;

    while ((next = findFirstDelimiter(source, delimiter, delimiterCount, prev)) != std::string::npos) {
        if (keepEmpty || (next - prev != 0)) {
            results.push_back(std::string(source.substr(prev, next - prev)));
        }
        prev = next + 1;
    }

    if (prev < source.size()) {
        results.push_back(std::string(source.substr(prev)));
    }

    return results;
}

int find(char* list[], unsigned int listSize, const std::string& item)
{
    for (unsigned int i = 0; i < listSize; i++) {
        if (std::string(list[i]).compare(item) == 0)
            return int(i);
    }
    return -1; // not found
}
}
}
