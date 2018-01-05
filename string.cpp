#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <string>
#include <string_view>

namespace infra::str {

std::string lowercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), std::tolower);
    return result;
}

std::string uppercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), std::toupper);
    return result;
}

void lowercaseInPlace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), std::tolower);
}

void uppercaseInPlace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), std::toupper);
}

bool startsWith(std::string_view aString, std::string_view search)
{
    return aString.compare(0, search.size(), search) == 0;
}

bool endsWith(std::string_view aString, std::string_view search)
{
    if (search.size() > aString.size()) {
        return false;
    }

    return aString.rfind(search) == (aString.size() - search.size());
}

std::string_view trimmedView(std::string_view str)
{
    if (str.empty()) {
        return str;
    }

    auto begin = str.find_first_not_of(" \t\r\n");
    auto end   = str.find_last_not_of(" \t\r\n");

    if (begin == std::string_view::npos && end == std::string_view::npos) {
        return std::string_view();
    }

    assert(begin != std::string_view::npos);
    assert(end != std::string_view::npos);
    assert(begin <= end);

    return std::string_view(&str[begin], (end + 1) - begin);
}

std::string trim(std::string_view str)
{
    auto trimmed = trimmedView(str);
    return std::string(trimmed.begin(), trimmed.end());
}

void trimInPlace(std::string& str)
{
    auto trimmed = trimmedView(str);
    if (trimmed.data() == str.data() && trimmed.size() == str.size()) {
        // no trimming was needed
        return;
    }

    str.assign(trimmed.begin(), trimmed.end());
}
}
