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
}
