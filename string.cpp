#include "infra/string.h"
#include "infra/exception.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <string>
#include <string_view>

namespace infra::str {

const char* Splitter::WhiteSpaceSeparator = "\t\n\r\f";

bool containsValidInteger(std::string_view str)
{
    std::string s(str);

    char* end = nullptr;
    std::strtol(s.c_str(), &end, 10);
    return (end != nullptr && *end == 0);
}

bool containsValidFloatingPoint(std::string_view str)
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
            if (isDelimiter(str[i], delimiter)) {
                ++matchLength;
            } else {
                if (matchLength > 0) {
                    if (matchLength != i) {
                        // if matchlength == i, we have only had delimeter data
                        applySplitOptions(std::string_view(start, length), opt, tokens);
                    }
                    length      = 1;
                    matchLength = 0;
                    start       = &str[i];
                    continue;
                }

                ++length;
                matchLength = 0;
            }
        }

        if (length > 0) {
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

Splitter::Splitter(std::string_view src, std::string sep)
: _src(src)
, _sep(std::move(sep))
{
}

Splitter::const_iterator::const_iterator(const Splitter& sp)
: _splitter(&sp)
, _pos(0)
{
    // skip initial delimiters
    _splitter->skipDelimiters(_pos);
    ++*this;
}

Splitter::const_iterator::const_iterator()
: _splitter(nullptr)
{
}

Splitter::const_iterator& Splitter::const_iterator::operator++() noexcept
{
    if (_pos != std::string_view::npos) {
        _value = _splitter->next(_pos);
    } else {
        _splitter = nullptr;
    }
    return *this;
}

Splitter::const_iterator Splitter::const_iterator::operator++(int) noexcept
{
    Splitter::const_iterator result(*this);
    ++(*this);
    return result;
}

Splitter::const_iterator::reference Splitter::const_iterator::operator*() const noexcept
{
    return _value;
}

Splitter::const_iterator::pointer Splitter::const_iterator::operator->() const noexcept
{
    return &_value;
}

bool Splitter::const_iterator::operator!=(const Splitter::const_iterator& other) const noexcept
{
    return _splitter != other._splitter;
}

Splitter::const_iterator Splitter::begin() const noexcept
{
    return Splitter::const_iterator(*this);
}

Splitter::const_iterator Splitter::end() const noexcept
{
    return Splitter::const_iterator();
}

void Splitter::skipDelimiters(size_t& pos) const noexcept
{
    pos = _src.find_first_not_of(_sep, pos);
}

std::string_view Splitter::next(size_t& pos) const noexcept
{
    const auto endOfCurrent = _src.substr(pos).find_first_of(_sep);
    if (endOfCurrent != std::string_view::npos && endOfCurrent != _src.size()) {
        // skip all occurrences of the delimeter
        const auto startOfNext = _src.substr(pos + endOfCurrent).find_first_not_of(_sep);
        const auto old_pos     = pos;
        pos += endOfCurrent;

        if (startOfNext != std::string_view::npos) {
            pos += startOfNext;
        } else {
            pos = std::string_view::npos;
        }

        return _src.substr(old_pos, endOfCurrent);
    } else {
        auto result = _src.substr(pos);
        pos         = std::string_view::npos;
        return result;
    }
}
}
