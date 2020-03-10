//! String utility functions
#pragma once

#include <algorithm>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "infra/enumflags.h"
#include "infra/typetraits-private.h"
#include "infra/typetraits.h"

namespace inf::str {

/*!
 * \brief check if a string contains a valid integer value
 * \param str string to parse
 * \return the parsing result
 */
bool contains_valid_integer(std::string_view str);

/*!
 * \brief check if a string contains a valid float value
 * \param str string to parse
 * \return the parsing result
 */
bool contains_valid_float(std::string_view str);

/*!
 * \brief check if a string contains a valid double value
 * \param str string to parse
 * \return the parsing result
 */
bool contains_valid_double(std::string_view str);

/*!
 * \brief string conversion to signed 32bit integer
 * \param str string to parse
 * \return optional int32_t that contains a value if the string contained a valid integer
 */
std::optional<int32_t> to_int32(std::string_view str) noexcept;

/*!
 * \brief string conversion to signed 32bit integer
 * \param str string to parse
 * \return that int32 value
 * \throw InvalidArgument if the input string does not contain a valid integer
 */
int32_t to_int32_value(std::string_view str);

/*!
 * \brief string conversion to signed 32bit integer
 * \param str string to parse
 * \return optional uint32_t that contains a value if the string contained a valid integer
 */
std::optional<uint32_t> to_uint32(std::string_view str) noexcept;

/*!
 * \brief string conversion to signed 32bit integer
 * \param str string to parse
 * \return that uint32 value
 * \throw InvalidArgument if the input string does not contain a valid integer
 */
uint32_t to_uint32_value(std::string_view str);

/*!
 * \brief string conversion to signed 64bit integer
 * \param str string to parse
 * \return optional int64_t that contains a value if the string contained a valid integer
 */
std::optional<int64_t> to_int64(std::string_view str) noexcept;

/*!
 * \brief string conversion to signed 64bit integer
 * \param str string to parse
 * \return the int64 value
 * \throw InvalidArgument if the input string does not contain a valid integer
 */
int64_t to_int64_value(std::string_view str);

/*!
 * \brief string conversion to signed 64bit integer
 * \param str string to parse
 * \return optional uint64_t that contains a value if the string contained a valid integer
 */
std::optional<uint64_t> to_uint64(std::string_view str) noexcept;

/*!
 * \brief string conversion to signed 64bit integer
 * \param str string to parse
 * \return the uint64 value
 * \throw InvalidArgument if the input string does not contain a valid integer
 */
uint64_t to_uint64_value(std::string_view str);

/*!
 * \brief string conversion to float
 * \param str string to parse
 * \return optional float that contains a value if the string contained a valid integer
 */
std::optional<float> to_float(std::string_view str) noexcept;

/*!
 * \brief string conversion to float
 * \param str string to parse
 * \return the foat value
 * \throw InvalidArgument if the input string does not contain a valid float
 */
float to_float_value(std::string_view str);

/*!
 * \brief string conversion to double
 * \param str string to parse
 * \return the float value or 0.0 in case of a parsing error
 */
float to_float_value_zero_on_error(std::string_view str) noexcept;

/*!
 * \brief string conversion to double
 * \param str string to parse
 * \return optional double that contains a value if the string contained a valid integer
 */
std::optional<double> to_double(std::string_view str) noexcept;

/*!
 * \brief string conversion to double
 * \param str string to parse
 * \return the double value
 * \throw InvalidArgument if the input string does not contain a valid double
 */
double to_double_value(std::string_view str);

/*!
 * \brief string conversion to double
 * \param str string to parse
 * \return the double value or 0.0 in case of a parsing error
 */
double to_double_value_zero_on_error(std::string_view str) noexcept;

/*!
 * \brief templated version of the string to numeric functions
 * \param str string to parse
 * \tparam T the numeric type
 * \return optional<T> that contains a value if the string contained a valid number
 */
template <typename T>
std::optional<T> to_numeric(std::string_view str) noexcept
{
    if constexpr (std::is_same_v<int32_t, T>) {
        return to_int32(str);
    } else if constexpr (std::is_same_v<uint32_t, T>) {
        return to_uint32(str);
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return to_int64(str);
    } else if constexpr (std::is_same_v<uint64_t, T>) {
        return to_uint64(str);
    } else if constexpr (std::is_same_v<float, T>) {
        return to_float(str);
    } else if constexpr (std::is_same_v<double, T>) {
        return to_double(str);
    } else {
        static_assert(dependent_false_v<T>, "Invalid type provided");
    }
}

/*!
 * \brief templated version of the string to numeric functions
 * \param str string to parse
 * \tparam T the numeric type
 * \return optional<T> that contains a value if the string contained a valid number
 * \throw InvalidArgument if the input string does not contain a valid number
 */
template <typename T>
T to_numeric_value(std::string_view str)
{
    if constexpr (std::is_same_v<int32_t, T>) {
        return to_int32_value(str);
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return to_int64_value(str);
    } else if constexpr (std::is_same_v<float, T>) {
        return to_float_value(str);
    } else if constexpr (std::is_same_v<double, T>) {
        return to_double_value(str);
    } else {
        static_assert(dependent_false_v<T>, "Invalid type provided");
    }
}

/*!
 * \brief case insensitive string comparision
 * \param str1 to compare
 * \param str2 to compare
 * \return true if the strings are considered equal
 */
bool iequals(std::string_view str1, std::string_view str2);

/*!
 * \brief case insensitive string comparision
 * \param str1 to compare
 * \param str2 to compare
 * \return 0 if the strings are considered equal
           < 0 str1 is less than str2
           > 0 str1 is greater than str2
 */
int icompare(std::string_view str1, std::string_view str2);

/*!
 * \brief replace all occurences of the toSearch string with toReplace modifying aString
 * \param str in which the replacement will occur
 * \param toSearch the search string
 * \param toReplace the replacement string
 */
void replace_in_place(std::string& str, std::string_view toSearch, std::string_view toReplace);
void replace_in_place(std::string& aString, char toSearch, char toReplace);

/*!
 * \brief replace all occurences of the toSearch string with toReplace
 * \param str to search for occurences of the search string
 * \param toSearch the search string
 * \param toReplace the replacement string
 * \return resulting string with the replacements
 */
[[nodiscard]] std::string replace(std::string_view str, std::string_view toSearch, std::string_view toReplace);

/*!
 * \brief transform string to lowercase
 * \param str input string
 * \return lowercased input string
 */
[[nodiscard]] std::string lowercase(std::string_view str);

/*!
 * \brief transform string to uppercase
 * \param str input string
 * \return uppercased input string
 */
[[nodiscard]] std::string uppercase(std::string_view str);

/*!
 * \brief transform string to lowercase (in place version)
 * \param str input string that will be lowercased
 */
void lowercase_in_place(std::string& str);

/*!
 * \brief transform string to uppercase (in place version)
 * \param str input string that will be uppercased
 */
void uppercase_in_place(std::string& str);

/*!
 * \brief check if a string starts with the provided string
 * \param str input string
 * \param search string
 * \return true is the input string starts with the search string
 */
bool starts_with(std::string_view str, std::string_view search);

/*!
 * \brief check if a string starts with the provided string ignoring casing
 * \param str input string
 * \param search string
 * \return true is the input string starts with the search string
 */
bool starts_with_ignore_case(std::string_view str, std::string_view search);

/*!
 * \brief check if a string ends with the provided string
 * \param str input string
 * \param search string
 * \return true is the input string ends with the search string
 */
bool ends_with(std::string_view str, std::string_view search);

/*!
 * \brief check if a string ends with the provided string ignoring casing
 * \param str input string
 * \param search string
 * \return true is the input string ends with the search string
 */
bool ends_with_ignore_case(std::string_view aString, std::string_view search);

/*!
 * \brief create a trimmed view to a astring
 * \param str input string
 * \return trimmed view of the input string
 * \note it is important that the lifetime of the returned view is shorter than that of the input string
 */
[[nodiscard]] std::string_view trimmed_view(std::string_view str);

/*!
 * \brief create a trimmed copy of a astring
 * \param str input string
 * \return trimmed copy of the input string
 */
[[nodiscard]] std::string trim(std::string_view str);
void trim_in_place(std::string& str);

/*!
 * \brief Join items in the container with the provided join string
 * \param items container containing the items to join (needs to be iterable)
 * \param joinString that will be placed between each item
 * \tparam Container the container type
 * \details The items in the container need to be convertible to std::string_view
 *          using a static_cast or need to be streamable to an std::ostream
 * \code
 * str::join(std::vector<std::string>({"one", "two", "three"}), ", ") == "one, two, three"
 * \endcode
 */

template <typename Container>
std::string join(const Container& items, std::string_view joinString)
{
    using ValueType = typename Container::value_type;
    static_assert(can_cast_to_string_view_v<ValueType> || is_streamable_v<ValueType>,
                  "Items to join should be streamable or convertible to string_view");

    std::string result;

    if (items.empty()) {
        return result;
    }

    if constexpr (can_cast_to_string_view_v<ValueType>) {
        // Join implementation for objects that implement: std::basic_string::operator basic_string_view
        // This implementation is roughly 10 times faster than the ostream version for strings

        size_t inputSize = 0;
        for (auto& item : items) {
            inputSize += static_cast<std::string_view>(item).size();
        }

        size_t resultSize = inputSize + ((items.size() - 1) * joinString.size());
        result.reserve(resultSize);

        for (auto& item : items) {
            result += static_cast<std::string_view>(item);

            if (result.size() < resultSize) {
                result += joinString;
            }
        }
    } else if constexpr (is_streamable_v<ValueType>) {
        // Join implementation for objects that have a streaming operator implemented
        std::ostringstream ss;
        for (auto iter = items.cbegin(); iter != items.cend(); ++iter) {
            ss << *iter;
            if (std::next(iter) != items.cend()) {
                ss << joinString;
            }
        }

        result = ss.str();
    } else {
        static_assert(dependent_false_v<ValueType>, "invalid invocation");
    }

    return result;
}

template <typename Container>
std::string join(const Container& items, char joinCharacter)
{
    return join(items, std::string_view(&joinCharacter, 1));
}

/*!
 * \brief Join items in the container with the provided join string with custom string conversion
 * \param items container containing the items to join (needs to be iterable)
 * \param joinString that will be placed between each item
 * \param cb callable that will be called for each item
 * \tparam Container the container type
 * \tparam ToStringCb the callable type to convert the items to string
 * \details the return type of the callable should be streamable to an std::ostream
 *          using a static_cast or need to be streamable to an std::ostream\n
 *          e.g. join a vector of integers:
 * \code
 *  str::join({1, 2, 3}, ", ", str::toInt32Value) == "1, 2, 3"
 * \endcode
 */
template <typename Container, typename ToStringCb>
std::string join(const Container& items, std::string_view joinString, ToStringCb&& cb)
{
    std::ostringstream ss;
    for (auto iter = begin(items); iter != end(items); ++iter) {
        ss << cb(*iter);
        if (std::next(iter) != end(items)) {
            ss << joinString;
        }
    }

    return ss.str();
}

/*! Split options that can be passed to splitting functions */
enum class SplitOpt
{
    NoEmpty                    = 1 << 0, /**< Don't include empty substrings in the results */
    Trim                       = 1 << 1, /**< Trim the substrings */
    DelimiterIsCharacterArray  = 1 << 2, /**< Treat the delimiter as an array of characters, any occurence is used to split */
    JoinAdjacentCharDelimeters = 1 << 3, /**< Adjacent character delimeters are joined together */
};

inline constexpr Flags<SplitOpt> operator|(SplitOpt lhs, SplitOpt rhs) noexcept
{
    return Flags<SplitOpt>() | lhs | rhs;
}

//! Use this combination of flags to get the same behavior as strtok
static inline constexpr Flags<SplitOpt> StrTokFlags = SplitOpt::NoEmpty | SplitOpt::DelimiterIsCharacterArray | SplitOpt::JoinAdjacentCharDelimeters;

/*! \brief Splits the passed strings into substrings using the delimeter character
 * \param str the string to split
 * \param delimiter the delimiter character on which to split
 * \param opt optional splitting options
 * \details When splitting on a single character, use this overload as it is slightly more efficient then using a string delimiter
 */
std::vector<std::string> split(std::string_view str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());
/*! Splits the passed strings into substrings using the delimeter character */
std::vector<std::string> split(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

std::vector<std::string_view> split_view(std::string_view str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());
std::vector<std::string_view> split_view(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

template <typename ResultType, typename ConversionFunc>
std::vector<ResultType> split_to(std::string_view str, char delimiter, ConversionFunc convFunc, Flags<SplitOpt> opt = Flags<SplitOpt>())
{
    auto splitted = split_view(str, delimiter, opt);

    std::vector<ResultType> result;
    result.reserve(splitted.size());
    for (auto& v : splitted) {
        result.emplace_back(convFunc(v));
    }

    return result;
}

template <typename ResultType, typename ConversionFunc>
std::vector<ResultType> split_to(std::string_view str, std::string_view delimiter, ConversionFunc convFunc, Flags<SplitOpt> opt = Flags<SplitOpt>())
{
    auto splitted = split_view(str, delimiter, opt);

    std::vector<ResultType> result;
    result.reserve(splitted.size());
    for (auto& v : splitted) {
        result.emplace_back(convFunc(v));
    }

    return result;
}

class Splitter
{
public:
    using value_type      = std::string_view;
    using pointer         = const value_type*;
    using reference       = const value_type&;
    using iterator        = std::vector<std::string_view>::iterator;
    using difference_type = std::vector<std::string_view>::difference_type;

    Splitter(std::string_view src, std::string_view sep, Flags<SplitOpt> opt = Flags<SplitOpt>()) noexcept
    : _result(str::split_view(src, sep, opt))
    {
    }

    Splitter(std::string_view src, char sep, Flags<SplitOpt> opt = Flags<SplitOpt>()) noexcept
    : _result(str::split_view(src, sep, opt))
    {
    }

    auto begin() const noexcept
    {
        return _result.begin();
    }

    auto end() const noexcept
    {
        return _result.end();
    }

private:
    std::vector<std::string_view> _result;
};

void ellipsize_in_place(std::string& str, int maxLength);
[[nodiscard]] std::string ellipsize(std::string_view str, int maxLength);

//class Splitter
//{
//public:
//    static const char* WhiteSpaceSeparator;

//    Splitter(std::string_view src, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

//    class const_iterator
//    {
//    public:
//        using value_type        = std::string_view;
//        using pointer           = const value_type*;
//        using reference         = const value_type&;
//        using iterator_category = std::forward_iterator_tag;
//        using difference_type   = int;

//        explicit const_iterator(const Splitter& sp) noexcept;
//        explicit const_iterator() noexcept;

//        const_iterator& operator++() noexcept;
//        const_iterator operator++(int) noexcept;

//        reference operator*() const noexcept;
//        pointer operator->() const noexcept;

//        bool operator!=(const const_iterator& other) const noexcept;

//    private:
//        const Splitter* _splitter;
//        std::string_view::size_type _pos;
//        std::string_view _value;
//    };

//    const_iterator begin() const noexcept;
//    const_iterator end() const noexcept;

//private:
//    std::string_view next(std::string_view::size_type& pos) const noexcept;

//    std::string_view _src;
//    std::string _delimiter;
//    Flags<SplitOpt> _splitopts;
//};

} // namespace inf::str
