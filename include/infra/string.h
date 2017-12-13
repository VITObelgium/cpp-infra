#pragma once

#include <algorithm>
#include <string>
#include <string_view>

namespace infra::str {

inline std::string lowercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), tolower);
    return result;
}

inline bool startsWith(std::string_view aString, std::string_view search)
{
    return aString.compare(0, search.size(), search) == 0;
}

inline bool endsWith(std::string_view aString, std::string_view search)
{
    if (search.size() > aString.size()) {
        return false;
    }

    return aString.rfind(search) == (aString.size() - search.size());
}

inline std::vector<std::string> tokenize(std::string_view str, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    size_t pos   = 0;
    size_t index = 0;

    while ((pos = str.find(delimiter, index)) != std::string::npos) {
        tokens.emplace_back(str.substr(index, pos - index));
        index = pos + delimiter.size();
    }

    if (index < str.size()) {
        tokens.emplace_back(str.substr(index));
    }

    return tokens;
}
}
