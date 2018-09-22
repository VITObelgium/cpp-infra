#include "infra/string.h"
#include "infra/exception.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

namespace inf::str {

bool containsValidInteger(std::string_view str)
{
    std::string s(str);

    char* end = nullptr;
    std::strtol(s.c_str(), &end, 10);
    return (end != nullptr && *end == 0);
}

bool containsValidFloat(std::string_view str)
{
    std::string s(str);

    char* end = nullptr;
    std::strtof(s.c_str(), &end);
    return (end != nullptr && *end == 0);
}

bool containsValidDouble(std::string_view str)
{
    std::string s(str);

    char* end = nullptr;
    std::strtod(s.c_str(), &end);
    return (end != nullptr && *end == 0);
}

std::optional<int32_t> toInt32(std::string_view str) noexcept
{
    std::string s(str);

    char* end   = nullptr;
    long result = std::strtol(s.c_str(), &end, 10);
    if (end == s.c_str()) {
        return std::optional<int32_t>();
    }

    return result;
}

int32_t toInt32Value(std::string_view str)
{
    auto optval = toInt32(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to int32", str);
    }

    return *optval;
}

std::optional<int64_t> toInt64(std::string_view str) noexcept
{
    std::string s(str);

    char* end      = nullptr;
    int64_t result = std::strtoll(s.c_str(), &end, 10);
    if (end == s.c_str()) {
        return std::optional<int64_t>();
    }

    return result;
}

int64_t toInt64Value(std::string_view str)
{
    auto optval = toInt64(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to int64", str);
    }

    return *optval;
}

std::optional<float> toFloat(std::string_view str) noexcept
{
    std::string s(str);

    char* end    = nullptr;
    float result = std::strtof(s.c_str(), &end);
    if (end == s.c_str()) {
        return std::optional<float>();
    }

    return result;
}

float toFloatValue(std::string_view str)
{
    auto optval = toFloat(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to float", str);
    }

    return *optval;
}

std::optional<double> toDouble(std::string_view str) noexcept
{
    std::string s(str);

    char* end     = nullptr;
    double result = std::strtod(s.c_str(), &end);
    if (end == s.c_str()) {
        return std::optional<double>();
    }

    return result;
}

double toDoubleValue(std::string_view str)
{
    auto optval = toDouble(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to double", str);
    }

    return *optval;
}

bool iequals(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size()) {
        return false;
    }

    for (size_t i = 0; i < str1.size(); ++i) {
        if (tolower(str1[i]) != tolower(str2[i])) {
            return false;
        }
    }

    return true;
}

void replaceInPlace(std::string& aString, std::string_view toSearch, std::string_view toReplace)
{
    size_t startPos = 0;
    size_t foundPos;

    while (std::string::npos != (foundPos = aString.find(toSearch, startPos))) {
        aString.replace(foundPos, toSearch.length(), toReplace);
        startPos = foundPos + toReplace.size();
    }
}

std::string replace(std::string_view aString, std::string_view toSearch, std::string_view toReplace)
{
    std::string result(aString);
    replaceInPlace(result, toSearch, toReplace);
    return result;
}

std::string lowercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), [](char c) {
        return static_cast<char>(::tolower(c));
    });
    return result;
}

std::string uppercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), [](char c) {
        return static_cast<char>(::toupper(c));
    });
    return result;
}

void lowercaseInPlace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) {
        return static_cast<char>(::tolower(c));
    });
}

void uppercaseInPlace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) {
        return static_cast<char>(::toupper(c));
    });
}

bool startsWith(std::string_view aString, std::string_view search)
{
    return aString.compare(0, search.size(), search) == 0;
}

bool startsWithIgnoreCase(std::string_view aString, std::string_view search)
{
    if (search.size() > aString.size()) {
        return false;
    }

    for (size_t i = 0; i < search.size(); ++i) {
        if (std::tolower(aString[i]) != std::tolower(search[i])) {
            return false;
        }
    }

    return true;
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

    if (sv.empty() && opt.is_set(SplitOpt::NoEmpty)) {
        return;
    }

    tokens.emplace_back(sv);
}

//static std::string_view applySplitOptions(std::string_view sv, Flags<SplitOpt> opt)
//{
//    if (opt.is_set(SplitOpt::Trim)) {
//        return trimmedView(sv);
//    }
//
//    return sv;
//}

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

static bool isDelimiter(char character, std::string_view delimiter)
{
    for (auto& del : delimiter) {
        if (del == character) {
            return true;
        }
    }

    return false;
}

std::vector<std::string_view> splitView(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt)
{
    std::vector<std::string_view> tokens;

    size_t length      = 0;
    size_t matchLength = 0;
    const char* start  = str.data();

    if (opt.is_set(SplitOpt::DelimiterIsCharacterArray)) {
        for (size_t i = 0; i < str.size(); ++i) {
            if (length == 0) {
                start = &str[i];
            }

            matchLength = 0;
            if (isDelimiter(str[i], delimiter)) {
                ++matchLength;

                if (opt.is_set(SplitOpt::JoinAdjacentCharDelimeters)) {
                    while (i + 1 < str.size() && isDelimiter(str[i + 1], delimiter)) {
                        ++matchLength;
                        ++i;
                    }
                }
            }

            if (matchLength > 0) {
                //if (matchLength != str.size()) {
                // if matchlength == str size, we have only had delimeter data
                applySplitOptions(std::string_view(start, length), opt, tokens);
                length = 0;
                //}
            } else {
                ++length;
            }
        }

        if (length > 0) {
            applySplitOptions(std::string_view(start, length), opt, tokens);
        }

        if (matchLength > 0) {
            applySplitOptions(std::string_view(start, length), opt, tokens);
        }
    } else {
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

std::vector<std::string> split(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt)
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

//Splitter::Splitter(std::string_view src, std::string_view delimiter, Flags<SplitOpt> opt)
//: _src(src)
//, _delimiter(delimiter)
//, _splitopts(opt)
//{
//}

//Splitter::const_iterator::const_iterator(const Splitter& sp) noexcept
//: _splitter(&sp)
//, _pos(0)
//{
//    ++*this;
//}

//Splitter::const_iterator::const_iterator() noexcept
//: _splitter(nullptr)
//, _pos(std::string_view::npos)
//{
//}

//Splitter::const_iterator& Splitter::const_iterator::operator++() noexcept
//{
//    if (_pos != std::string_view::npos) {
//        _value = _splitter->next(_pos);
//    }

//    if (_pos == std::string_view::npos) {
//        _splitter = nullptr;
//    }

//    return *this;
//}

//Splitter::const_iterator Splitter::const_iterator::operator++(int) noexcept
//{
//    Splitter::const_iterator result(*this);
//    ++(*this);
//    return result;
//}

//Splitter::const_iterator::reference Splitter::const_iterator::operator*() const noexcept
//{
//    return _value;
//}

//Splitter::const_iterator::pointer Splitter::const_iterator::operator->() const noexcept
//{
//    return &_value;
//}

//bool Splitter::const_iterator::operator!=(const Splitter::const_iterator& other) const noexcept
//{
//    std::cout << _splitter << " - " << other._splitter << " - " << _pos << " - " << other._pos << " - " << std::endl;
//    return _splitter != other._splitter || _pos != other._pos;
//}

//Splitter::const_iterator Splitter::begin() const noexcept
//{
//    return Splitter::const_iterator(*this);
//}

//Splitter::const_iterator Splitter::end() const noexcept
//{
//    return Splitter::const_iterator();
//}

//std::string_view Splitter::next(std::string_view::size_type& pos) const noexcept
//{
//    std::cout << "Next: " << pos << std::endl;

//    size_t length      = 0;
//    size_t matchLength = 0;
//    const char* start  = &_src[pos];
//    if (_splitopts.is_set(SplitOpt::DelimiterIsCharacterArray)) {
//        for (; pos < _src.size(); ++pos) {
//            if (isDelimiter(_src[pos], _delimiter)) {
//                ++matchLength;

//                if (_splitopts.is_set(SplitOpt::JoinAdjacentCharDelimeters)) {
//                    while (pos + 1 < _src.size() && isDelimiter(_src[pos + 1], _delimiter)) {
//                        ++matchLength;
//                        ++pos;
//                    }
//                }
//            }

//            if (matchLength > 0) {
//                if (_splitopts.is_set(SplitOpt::NoEmpty) && length == 0) {
//                    continue;
//                }

//                ++pos;
//                std::cout << "Next 1: " << length << std::endl;
//                return applySplitOptions(std::string_view(start, length), _splitopts);
//            } else {
//                ++length;
//            }
//        }

//        if (length > 0) {
//            return applySplitOptions(std::string_view(start, length), _splitopts);
//        } else if (matchLength > 0) {
//            return applySplitOptions(std::string_view(start, length), _splitopts);
//        } else if (pos != _src.size()) {
//            return applySplitOptions(std::string_view(start, length), _splitopts);
//        }

//        std::cout << "Next 2: npos " << std::endl;
//        pos = std::string_view::npos;
//        return std::string_view();
//    } else {
//        for (; pos < _src.size(); ++pos) {
//            if (_src[pos] == _delimiter[matchLength]) {
//                ++matchLength;

//                if (matchLength == _delimiter.size()) {
//                    ++pos;
//                    return applySplitOptions(std::string_view(start, length), _splitopts);
//                }
//            } else {
//                length += matchLength;
//                ++length;
//                matchLength = 0;
//            }
//        }

//        if (matchLength == _delimiter.size()) {
//            return applySplitOptions(std::string_view(), _splitopts);
//        } else if (length + matchLength > 0) {
//            return applySplitOptions(std::string_view(start, length), _splitopts);
//        }

//        //        if (length == 0 && matchLength == 0) {
//        //            applySplitOptions(std::string_view(), opt, tokens);
//        //        }
//        pos = std::string_view::npos;
//        return std::string_view();
//    }
//}
}
