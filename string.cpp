#include "infra/string.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <string>
#include <string_view>

namespace infra::str {

void replace(std::string& aString, std::string_view toSearch, std::string_view toReplace)
{
    size_t startPos = 0;
    size_t foundPos;

    while (std::string::npos != (foundPos = aString.find(toSearch, startPos))) {
        aString.replace(foundPos, toSearch.length(), toReplace);
        startPos = foundPos + toReplace.size();
    }
}

std::string lowercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), ::tolower);
    return result;
}

std::string uppercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), ::toupper);
    return result;
}

void lowercaseInPlace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void uppercaseInPlace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
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

static void applySplitOptions(std::string_view sv, Flags<SplitOpt> opt, std::vector<std::string_view>& tokens)
{
    if (opt.is_set(SplitOpt::Trim)) {
        sv = trimmedView(sv);
    }

    if (!opt.is_set(SplitOpt::NoEmpty) || !sv.empty()) {
        tokens.emplace_back(sv);
    }
}

std::vector<std::string_view> splitView(std::string_view str, char delimiter, Flags<SplitOpt> opt)
{
    std::vector<std::string_view> tokens;

    size_t length     = 0;
    const char* start = str.data();
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delimiter) {
            applySplitOptions(std::string_view(start, length), opt, tokens);
            length = 0;

            if (i + 1 < str.size()) {
                start = &str[i + 1];
            }
        } else {
            ++length;
        }
    }

    if (length > 0) {
        applySplitOptions(std::string_view(start, length), opt, tokens);
    }

    if (*start == delimiter) {
        applySplitOptions(std::string_view(), opt, tokens);
    }

    if (tokens.empty()) {
        applySplitOptions(str, opt, tokens);
    }

    return tokens;
}

std::vector<std::string_view> splitView(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt)
{
    std::vector<std::string_view> tokens;

    size_t length      = 0;
    size_t matchLength = 0;
    const char* start  = str.data();

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delimiter[matchLength]) {
            ++matchLength;

            if (matchLength == delimiter.size()) {
                applySplitOptions(std::string_view(start, length), opt, tokens);
                length      = 0;
                matchLength = 0;

                if (i + 1 < str.size()) {
                    start = &str[i + 1];
                }

                continue;
            }
        } else {
            length += matchLength;
            ++length;
            matchLength = 0;
        }
    }

    if (matchLength == delimiter.size()) {
        applySplitOptions(std::string_view(), opt, tokens);
    } else if (length + matchLength > 0) {
        applySplitOptions(std::string_view(start, length), opt, tokens);
    }

    if (length == 0 && matchLength == 0) {
        applySplitOptions(std::string_view(), opt, tokens);
    }

    if (tokens.empty()) {
        applySplitOptions(std::string_view(start, length), opt, tokens);
    }

    return tokens;
}

std::vector<std::string> split(std::string_view str, char delimiter, Flags<SplitOpt> opt)
{
    auto splitted = splitView(str, delimiter, opt);

    std::vector<std::string> tokens;
    tokens.reserve(splitted.size());
    for (auto& v : splitted) {
        tokens.emplace_back(begin(v), end(v));
    }

    return tokens;
}

std::vector<std::string> split(std::string_view str, const std::string& delimiter, Flags<SplitOpt> opt)
{
    auto splitted = splitView(str, delimiter, opt);

    std::vector<std::string> tokens;
    tokens.reserve(splitted.size());
    tokens.reserve(splitted.size());
    for (auto& v : splitted) {
        tokens.emplace_back(begin(v), end(v));
    }
    return tokens;
}
}
