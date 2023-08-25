#include "infra/string.h"
#include "infra/cast.h"
#include "infra/exception.h"

#include <cassert>
#include <cctype>

namespace inf::str {

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool contains_valid_integer(std::basic_string_view<CharT, Traits> str)
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end = nullptr;
    std::strtol(s.c_str(), &end, 10);
    return (end != nullptr && *end == 0);
}

bool contains_valid_integer(std::string_view str)
{
    return contains_valid_integer<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool contains_valid_float(std::basic_string_view<CharT, Traits> str)
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end = nullptr;
    std::strtof(s.c_str(), &end);
    return (end != nullptr && *end == 0);
}

bool contains_valid_float(std::string_view str)
{
    return contains_valid_float<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool contains_valid_double(std::basic_string_view<CharT, Traits> str)
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end = nullptr;
    std::strtod(s.c_str(), &end);
    return (end != nullptr && *end == 0);
}

bool contains_valid_double(std::string_view str)
{
    return contains_valid_double<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::optional<int32_t> to_int32(std::basic_string_view<CharT, Traits> str) noexcept
{
    std::basic_string<CharT, Traits> s(str);

    char* end   = nullptr;
    long result = std::strtol(s.c_str(), &end, 10);
    if (end == s.c_str()) {
        return std::optional<int32_t>();
    }

    return result;
}

std::optional<int32_t> to_int32(std::string_view str) noexcept
{
    return to_int32<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
int32_t to_int32_value(std::basic_string_view<CharT, Traits> str)
{
    auto optval = to_int32(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to int32", std::string_view(str));
    }

    return *optval;
}

int32_t to_int32_value(std::string_view str)
{
    return to_int32_value<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::optional<uint32_t> to_uint32(std::basic_string_view<CharT, Traits> str) noexcept
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end  = nullptr;
    long result = std::strtoul(s.c_str(), &end, 10);
    if (end == s.c_str()) {
        return std::optional<uint32_t>();
    }

    return result;
}

std::optional<uint32_t> to_uint32(std::string_view str) noexcept
{
    return to_uint32<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
uint32_t to_uint32_value(std::basic_string_view<CharT, Traits> str)
{
    auto optval = to_uint32(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to uint32", str);
    }

    return *optval;
}

uint32_t to_uint32_value(std::string_view str)
{
    return to_uint32_value<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::optional<int64_t> to_int64(std::basic_string_view<CharT, Traits> str) noexcept
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end     = nullptr;
    int64_t result = std::strtoll(s.c_str(), &end, 10);
    if (end == s.c_str()) {
        return std::optional<int64_t>();
    }

    return result;
}

std::optional<int64_t> to_int64(std::string_view str) noexcept
{
    return to_int64<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
int64_t to_int64_value(std::basic_string_view<CharT, Traits> str)
{
    auto optval = to_int64(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to int64", str);
    }

    return *optval;
}

int64_t to_int64_value(std::string_view str)
{
    return to_int64_value<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::optional<uint64_t> to_uint64(std::basic_string_view<CharT, Traits> str) noexcept
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end     = nullptr;
    int64_t result = std::strtoull(s.c_str(), &end, 10);
    if (end == s.c_str()) {
        return std::optional<uint64_t>();
    }

    return result;
}

std::optional<uint64_t> to_uint64(std::string_view str) noexcept
{
    return to_uint64<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
uint64_t to_uint64_value(std::basic_string_view<CharT, Traits> str)
{
    auto optval = to_uint64(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to uint64", str);
    }

    return *optval;
}

uint64_t to_uint64_value(std::string_view str)
{
    return to_uint64_value<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::optional<float> to_float(std::basic_string_view<CharT, Traits> str) noexcept
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end   = nullptr;
    float result = std::strtof(s.c_str(), &end);
    if (end == s.c_str()) {
        return std::optional<float>();
    }

    return result;
}

std::optional<float> to_float(std::string_view str) noexcept
{
    return to_float<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
float to_float_value(std::basic_string_view<CharT, Traits> str)
{
    auto optval = to_float(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to float", str);
    }

    return *optval;
}

float to_float_value(std::string_view str)
{
    return to_float_value<char>(str);
}

float to_float_value_zero_on_error(std::string_view str) noexcept
{
    return to_float(str).value_or(0.f);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::optional<double> to_double(std::basic_string_view<CharT, Traits> str) noexcept
{
    std::basic_string<CharT, Traits> s(str);

    CharT* end    = nullptr;
    double result = std::strtod(s.c_str(), &end);
    if (end == s.c_str()) {
        return std::optional<double>();
    }

    return result;
}

std::optional<double> to_double(std::string_view str) noexcept
{
    return to_double<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
double to_double_value(std::basic_string_view<CharT, Traits> str)
{
    auto optval = to_double(str);
    if (!optval.has_value()) {
        throw InvalidArgument("Failed to convert '{}' to double", str);
    }

    return *optval;
}

double to_double_value(std::string_view str)
{
    return to_double_value<char>(str);
}

double to_double_value_zero_on_error(std::string_view str) noexcept
{
    return to_double(str).value_or(0.0);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool iequals(std::basic_string_view<CharT, Traits> str1, std::basic_string_view<CharT, Traits> str2)
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

bool iequals(std::string_view str1, std::string_view str2)
{
    return iequals<char>(str1, str2);
}

#if __cplusplus > 201703L
bool iequals(std::u8string_view str1, std::u8string_view str2)
{
    return iequals<char8_t>(str1, str2);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
int icompare(std::basic_string_view<CharT, Traits> str1, std::basic_string_view<CharT, Traits> str2)
{
    return lowercase(str1).compare(str2);
}

int icompare(std::string_view str1, std::string_view str2)
{
    return icompare<char>(str1, str2);
}

#if __cplusplus > 201703L
int icompare(std::u8string_view str1, std::u8string_view str2)
{
    return icompare<char8_t>(str1, str2);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
void replace_in_place(std::basic_string<CharT, Traits>& aString, std::basic_string_view<CharT, Traits> toSearch, std::basic_string_view<CharT, Traits> toReplace)
{
    size_t startPos = 0;
    size_t foundPos;

    while (std::basic_string<CharT, Traits>::npos != (foundPos = aString.find(toSearch, startPos))) {
        aString.replace(foundPos, toSearch.length(), toReplace);
        startPos = foundPos + toReplace.size();
    }
}

void replace_in_place(std::string& aString, std::string_view toSearch, std::string_view toReplace)
{
    return replace_in_place<char>(aString, toSearch, toReplace);
}

#if __cplusplus > 201703L
void replace_in_place(std::u8string& aString, std::u8string_view toSearch, std::u8string_view toReplace)
{
    return replace_in_place<char8_t>(aString, toSearch, toReplace);
}

#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
void replace_in_place(std::basic_string<CharT, Traits>& aString, CharT toSearch, CharT toReplace)
{
    const size_t length = aString.size();

    for (size_t i = 0; i < length; ++i) {
        if (aString[i] == toSearch) {
            aString[i] = toReplace;
        }
    }
}

void replace_in_place(std::string& aString, char toSearch, char toReplace)
{
    return replace_in_place<char>(aString, toSearch, toReplace);
}

#if __cplusplus > 201703L
void replace_in_place(std::u8string& aString, char8_t toSearch, char8_t toReplace)
{
    return replace_in_place<char8_t>(aString, toSearch, toReplace);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] std::basic_string<CharT, Traits> replace(std::basic_string_view<CharT, Traits> aString, std::basic_string_view<CharT, Traits> toSearch, std::basic_string_view<CharT, Traits> toReplace)
{
    std::basic_string<CharT, Traits> result(aString);
    replace_in_place(result, toSearch, toReplace);
    return result;
}

[[nodiscard]] std::string replace(std::string_view aString, std::string_view toSearch, std::string_view toReplace)
{
    return replace<char>(aString, toSearch, toReplace);
}

#if __cplusplus > 201703L
[[nodiscard]] std::u8string replace(std::u8string_view aString, std::u8string_view toSearch, std::u8string_view toReplace)
{
    return replace<char8_t>(aString, toSearch, toReplace);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] std::basic_string<CharT, Traits> lowercase(std::basic_string_view<CharT, Traits> str)
{
    std::basic_string<CharT, Traits> result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), [](char c) {
        return static_cast<char>(::tolower(c));
    });
    return result;
}

[[nodiscard]] std::string lowercase(std::string_view str)
{
    return lowercase<char>(str);
}

#if __cplusplus > 201703L
[[nodiscard]] std::u8string lowercase(std::u8string_view str)
{
    return lowercase<char8_t>(str);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] std::basic_string<CharT, Traits> uppercase(std::basic_string_view<CharT, Traits> str)
{
    std::basic_string<CharT, Traits> result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), [](char c) {
        return static_cast<CharT>(::toupper(c));
    });
    return result;
}

[[nodiscard]] std::string uppercase(std::string_view str)
{
    return uppercase<char>(str);
}

#if __cplusplus > 201703L
[[nodiscard]] std::u8string uppercase(std::u8string_view str)
{
    return uppercase<char8_t>(str);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
void lowercase_in_place(std::basic_string<CharT, Traits>& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](CharT c) {
        return static_cast<CharT>(::tolower(c));
    });
}

void lowercase_in_place(std::string& str)
{
    return lowercase_in_place<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
void uppercase_in_place(std::basic_string<CharT, Traits>& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](CharT c) {
        return static_cast<CharT>(::toupper(c));
    });
}

void uppercase_in_place(std::string& str)
{
    uppercase_in_place<char>(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool starts_with(std::basic_string_view<CharT, Traits> aString, std::basic_string_view<CharT, Traits> search)
{
    return aString.compare(0, search.size(), search) == 0;
}

bool starts_with(std::string_view aString, std::string_view search)
{
    return starts_with<char>(aString, search);
}

#if __cplusplus > 201703L
bool starts_with(std::u8string_view aString, std::u8string_view search)
{
    return starts_with<char8_t>(aString, search);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool starts_with_ignore_case(std::basic_string_view<CharT, Traits> aString, std::basic_string_view<CharT, Traits> search)
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

bool starts_with_ignore_case(std::string_view aString, std::string_view search)
{
    return starts_with_ignore_case<char>(aString, search);
}

#if __cplusplus > 201703L
bool starts_with_ignore_case(std::u8string_view aString, std::u8string_view search)
{
    return starts_with_ignore_case<char8_t>(aString, search);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool ends_with(std::basic_string_view<CharT, Traits> aString, std::basic_string_view<CharT, Traits> search)
{
    if (search.size() > aString.size()) {
        return false;
    }

    return aString.rfind(search) == (aString.size() - search.size());
}

bool ends_with(std::string_view aString, std::string_view search)
{
    return ends_with<char>(aString, search);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool ends_with_ignore_case(std::basic_string_view<CharT, Traits> aString, std::basic_string_view<CharT, Traits> search)
{
    if (search.size() > aString.size()) {
        return false;
    }

    size_t searchIndex = 0;
    for (size_t i = aString.size() - search.size(); i < aString.size(); ++i) {
        if (std::tolower(aString[i]) != std::tolower(search[searchIndex++])) {
            return false;
        }
    }

    return true;
}

bool ends_with_ignore_case(std::string_view aString, std::string_view search)
{
    return ends_with_ignore_case<char>(aString, search);
}

#if __cplusplus > 201703L
bool ends_with_ignore_case(std::u8string_view aString, std::u8string_view search)
{
    return ends_with_ignore_case<char8_t>(aString, search);
}
#endif

static const char* whitespaces(char)
{
    return " \t\r\n";
}

#if __cplusplus > 201703L
static const char8_t* whitespaces(char8_t)
{
    return u8" \t\r\n";
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] std::basic_string_view<CharT, Traits> trimmed_view(std::basic_string_view<CharT, Traits> str)
{
    if (str.empty()) {
        return str;
    }

    auto begin = str.find_first_not_of(whitespaces(CharT()));
    auto end   = str.find_last_not_of(whitespaces(CharT()));

    if (begin == std::basic_string_view<CharT, Traits>::npos && end == std::basic_string_view<CharT, Traits>::npos) {
        return {};
    }

    assert((begin != std::basic_string_view<CharT, Traits>::npos));
    assert((end != std::basic_string_view<CharT, Traits>::npos));
    assert(begin <= end);

    return std::basic_string_view<CharT, Traits>(&str[begin], (end + 1) - begin);
}

[[nodiscard]] std::string_view trimmed_view(std::string_view str)
{
    return trimmed_view<char>(str);
}

#if __cplusplus > 201703L
[[nodiscard]] std::u8string_view trimmed_view(std::u8string_view str)
{
    return trimmed_view<char8_t>(str);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] std::basic_string<CharT, Traits> trim(std::basic_string_view<CharT, Traits> str)
{
    auto trimmed = trimmed_view(str);
    return std::basic_string<CharT, Traits>(trimmed.begin(), trimmed.end());
}

[[nodiscard]] std::string trim(std::string_view str)
{
    return trim<char>(str);
}

#if __cplusplus > 201703L
[[nodiscard]] std::u8string trim(std::u8string_view str)
{
    return trim<char8_t>(str);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
void trim_in_place(std::basic_string<CharT, Traits>& str)
{
    auto trimmed = trimmed_view(str);
    if (trimmed.data() == str.data() && trimmed.size() == str.size()) {
        // no trimming was needed
        return;
    }

    str.assign(trimmed.begin(), trimmed.end());
}

void trim_in_place(std::string& str)
{
    trim_in_place<char>(str);
}

#if __cplusplus > 201703L
void trim_in_place(std::u8string& str)
{
    trim_in_place<char8_t>(str);
}
#endif

namespace detail {
template <typename CharT, typename Traits = std::char_traits<CharT>>
void apply_split_options(std::basic_string_view<CharT, Traits> sv, Flags<SplitOpt> opt, std::vector<std::basic_string_view<CharT, Traits>>& tokens)
{
    if (opt.is_set(SplitOpt::Trim)) {
        sv = trimmed_view(sv);
    }

    if (sv.empty() && opt.is_set(SplitOpt::NoEmpty)) {
        return;
    }

    tokens.emplace_back(sv);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_delimiter(char character, std::basic_string_view<CharT, Traits> delimiter)
{
    for (auto& del : delimiter) {
        if (del == character) {
            return true;
        }
    }

    return false;
}
}

template <typename CharT>
std::vector<std::basic_string_view<CharT>> split_view(std::basic_string_view<CharT> str, char delimiter, Flags<SplitOpt> opt)
{
    std::vector<std::basic_string_view<CharT>> tokens;

    size_t length      = 0;
    const CharT* start = str.data();
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delimiter) {
            detail::apply_split_options(std::basic_string_view<CharT>(start, length), opt, tokens);
            length = 0;

            if (opt.is_set(SplitOpt::JoinAdjacentCharDelimeters)) {
                while (i + 1 < str.size() && str[i + 1] == delimiter) {
                    ++i;
                }
            }

            if (i + 1 < str.size()) {
                start = &str[i + 1];
            }
        } else {
            ++length;
        }
    }

    if (length > 0) {
        detail::apply_split_options(std::basic_string_view<CharT>(start, length), opt, tokens);
    }

    if (*start == delimiter) {
        detail::apply_split_options(std::basic_string_view<CharT>(), opt, tokens);
    }

    if (tokens.empty()) {
        detail::apply_split_options(str, opt, tokens);
    }

    return tokens;
}

std::vector<std::string_view> split_view(std::string_view str, char delimiter, Flags<SplitOpt> opt)
{
    return split_view<char>(str, delimiter, opt);
}

#if __cplusplus > 201703L
std::vector<std::u8string_view> split_view(std::u8string_view str, char8_t delimiter, Flags<SplitOpt> opt)
{
    return split_view<char8_t>(str, delimiter, opt);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::vector<std::basic_string_view<CharT, Traits>> split_view(std::basic_string_view<CharT, Traits> str, std::basic_string_view<CharT, Traits> delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>())
{
    std::vector<std::basic_string_view<CharT, Traits>> tokens;

    size_t length      = 0;
    size_t matchLength = 0;
    const CharT* start = str.data();

    if (opt.is_set(SplitOpt::DelimiterIsCharacterArray)) {
        for (size_t i = 0; i < str.size(); ++i) {
            if (length == 0) {
                start = &str[i];
            }

            matchLength = 0;
            if (detail::is_delimiter(str[i], delimiter)) {
                ++matchLength;

                if (opt.is_set(SplitOpt::JoinAdjacentCharDelimeters)) {
                    while (i + 1 < str.size() && detail::is_delimiter(str[i + 1], delimiter)) {
                        ++matchLength;
                        ++i;
                    }
                }
            }

            if (matchLength > 0) {
                // if (matchLength != str.size()) {
                //  if matchlength == str size, we have only had delimeter data
                detail::apply_split_options(std::basic_string_view<CharT, Traits>(start, length), opt, tokens);
                length = 0;
                //}
            } else {
                ++length;
            }
        }

        if (length > 0) {
            detail::apply_split_options(std::basic_string_view<CharT, Traits>(start, length), opt, tokens);
        }

        if (matchLength > 0) {
            detail::apply_split_options(std::basic_string_view<CharT, Traits>(start, length), opt, tokens);
        }
    } else {
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == delimiter[matchLength]) {
                ++matchLength;

                if (matchLength == delimiter.size()) {
                    detail::apply_split_options(std::basic_string_view<CharT, Traits>(start, length), opt, tokens);
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
            detail::apply_split_options(std::basic_string_view<CharT, Traits>(), opt, tokens);
        } else if (length + matchLength > 0) {
            detail::apply_split_options(std::basic_string_view<CharT, Traits>(start, length), opt, tokens);
        }

        if (length == 0 && matchLength == 0) {
            detail::apply_split_options(std::basic_string_view<CharT, Traits>(), opt, tokens);
        }
    }

    if (tokens.empty()) {
        detail::apply_split_options(std::basic_string_view<CharT, Traits>(start, length), opt, tokens);
    }

    return tokens;
}

std::vector<std::string_view> split_view(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt)
{
    return split_view<char>(str, delimiter, opt);
}

#if __cplusplus > 201703L
std::vector<std::u8string_view> split_view(std::u8string_view str, std::u8string_view delimiter, Flags<SplitOpt> opt)
{
    return split_view<char8_t>(str, delimiter, opt);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::vector<std::basic_string<CharT, Traits>> split(std::basic_string_view<CharT, Traits> str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>())
{
    auto splitted = split_view(str, delimiter, opt);

    std::vector<std::basic_string<CharT, Traits>> tokens;
    tokens.reserve(splitted.size());
    for (auto& v : splitted) {
        tokens.emplace_back(begin(v), end(v));
    }

    return tokens;
}

std::vector<std::string> split(std::string_view str, char delimiter, Flags<SplitOpt> opt)
{
    return split<char>(str, delimiter, opt);
}

#if __cplusplus > 201703L
std::vector<std::u8string> split(std::u8string_view str, char8_t delimiter, Flags<SplitOpt> opt)
{
    return split<char8_t>(str, delimiter, opt);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::vector<std::basic_string<CharT, Traits>> split(std::basic_string_view<CharT, Traits> str, std::basic_string_view<CharT, Traits> delimiter, Flags<SplitOpt> opt)
{
    auto splitted = split_view(str, delimiter, opt);

    std::vector<std::basic_string<CharT, Traits>> tokens;
    tokens.reserve(splitted.size());
    for (auto& v : splitted) {
        tokens.emplace_back(begin(v), end(v));
    }
    return tokens;
}

std::vector<std::string> split(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt)
{
    return split<char>(str, delimiter, opt);
}

#if __cplusplus > 201703L
std::vector<std::u8string> split(std::u8string_view str, std::u8string_view delimiter, Flags<SplitOpt> opt)
{
    return split<char8_t>(str, delimiter, opt);
}
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
void ellipsize_in_place(std::basic_string<CharT, Traits>& str, int maxLength)
{
    if (inf::truncate<int>(str.size()) > maxLength) {
        if (maxLength > 2) {
            str.resize(maxLength);
            str[maxLength - 3] = '.';
            str[maxLength - 2] = '.';
            str[maxLength - 1] = '.';
        } else {
            str.resize(maxLength);
        }
    }
}

void ellipsize_in_place(std::string& str, int maxLength)
{
    return ellipsize_in_place<char>(str, maxLength);
}

#if __cplusplus > 201703L
void ellipsize_in_place(std::u8string& str, int maxLength)
{
    return ellipsize_in_place<char8_t>(str, maxLength);
}
#endif

[[nodiscard]] std::string ellipsize(std::string_view str, int maxLength)
{
    std::string result(str);
    ellipsize_in_place(result, maxLength);
    return result;
}

#if __cplusplus > 201703L
[[nodiscard]] std::u8string ellipsize(std::u8string_view str, int maxLength)
{
    std::u8string result(str);
    ellipsize_in_place(result, maxLength);
    return result;
}
#endif

// static std::string_view applySplitOptions(std::string_view sv, Flags<SplitOpt> opt)
//{
//     if (opt.is_set(SplitOpt::Trim)) {
//         return trimmed_view(sv);
//     }
//
//     return sv;
// }

// Splitter::Splitter(std::string_view src, std::string_view delimiter, Flags<SplitOpt> opt)
//: _src(src)
//, _delimiter(delimiter)
//, _splitopts(opt)
//{
//}

// Splitter::const_iterator::const_iterator(const Splitter& sp) noexcept
//: _splitter(&sp)
//, _pos(0)
//{
//    ++*this;
//}

// Splitter::const_iterator::const_iterator() noexcept
//: _splitter(nullptr)
//, _pos(std::string_view::npos)
//{
//}

// Splitter::const_iterator& Splitter::const_iterator::operator++() noexcept
//{
//     if (_pos != std::string_view::npos) {
//         _value = _splitter->next(_pos);
//     }

//    if (_pos == std::string_view::npos) {
//        _splitter = nullptr;
//    }

//    return *this;
//}

// Splitter::const_iterator Splitter::const_iterator::operator++(int) noexcept
//{
//     Splitter::const_iterator result(*this);
//     ++(*this);
//     return result;
// }

// Splitter::const_iterator::reference Splitter::const_iterator::operator*() const noexcept
//{
//     return _value;
// }

// Splitter::const_iterator::pointer Splitter::const_iterator::operator->() const noexcept
//{
//     return &_value;
// }

// bool Splitter::const_iterator::operator!=(const Splitter::const_iterator& other) const noexcept
//{
//     std::cout << _splitter << " - " << other._splitter << " - " << _pos << " - " << other._pos << " - " << std::endl;
//     return _splitter != other._splitter || _pos != other._pos;
// }

// Splitter::const_iterator Splitter::begin() const noexcept
//{
//     return Splitter::const_iterator(*this);
// }

// Splitter::const_iterator Splitter::end() const noexcept
//{
//     return Splitter::const_iterator();
// }

// std::string_view Splitter::next(std::string_view::size_type& pos) const noexcept
//{
//     std::cout << "Next: " << pos << std::endl;

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
