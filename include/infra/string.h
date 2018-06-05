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
#include "infra/internal/traits.h"
#include "infra/typetraits.h"

namespace infra::str {

/*!
 * \brief check if a string contains a valid integer value
 * \param str string to parse
 * \return the parsing result
 */
bool containsValidInteger(std::string_view str);

/*!
 * \brief check if a string contains a valid float value
 * \param str string to parse
 * \return the parsing result
 */
bool containsValidFloat(std::string_view str);

/*!
 * \brief check if a string contains a valid double value
 * \param str string to parse
 * \return the parsing result
 */
bool containsValidDouble(std::string_view str);

/*!
 * \brief string conversion to signed 32bit integer
 * \param str string to parse
 * \return optional int32_t that contains a value if the string contained a valid integer
 */
std::optional<int32_t> toInt32(std::string_view str) noexcept;

/*!
 * \brief string conversion to signed 32bit integer
 * \param str string to parse
 * \return that int32 value
 * \throw InvalidArgument if the input string does not contain a valid integer
 */
int32_t toInt32Value(std::string_view str);

/*!
 * \brief string conversion to signed 64bit integer
 * \param str string to parse
 * \return optional int64_t that contains a value if the string contained a valid integer
 */
std::optional<int64_t> toInt64(std::string_view str) noexcept;

/*!
 * \brief string conversion to signed 64bit integer
 * \param str string to parse
 * \return the int64 value
 * \throw InvalidArgument if the input string does not contain a valid integer
 */
int64_t toInt64Value(std::string_view str);

/*!
 * \brief string conversion to float
 * \param str string to parse
 * \return optional float that contains a value if the string contained a valid integer
 */
std::optional<float> toFloat(std::string_view str) noexcept;

/*!
 * \brief string conversion to float
 * \param str string to parse
 * \return the foat value
 * \throw InvalidArgument if the input string does not contain a valid float
 */
float toFloatValue(std::string_view str);

/*!
 * \brief string conversion to double
 * \param str string to parse
 * \return optional double that contains a value if the string contained a valid integer
 */
std::optional<double> toDouble(std::string_view str) noexcept;

/*!
 * \brief string conversion to double
 * \param str string to parse
 * \return the double value
 * \throw InvalidArgument if the input string does not contain a valid double
 */
double toDoubleValue(std::string_view str);

/*!
 * \brief templated version of the string to numeric functions
 * \param str string to parse
 * \tparam T the numeric type
 * \return optional<T> that contains a value if the string contained a valid number
 */
template <typename T>
std::optional<T> toNumeric(std::string_view str) noexcept
{
    if constexpr (std::is_same_v<int32_t, T>) {
        return toInt32(str);
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return toInt64(str);
    } else if constexpr (std::is_same_v<float, T>) {
        return toFloat(str);
    } else if constexpr (std::is_same_v<double, T>) {
        return toDouble(str);
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
T toNumericValue(std::string_view str)
{
    if constexpr (std::is_same_v<int32_t, T>) {
        return toInt32Value(str);
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return toInt64Value(str);
    } else if constexpr (std::is_same_v<float, T>) {
        return toFloatValue(str);
    } else if constexpr (std::is_same_v<double, T>) {
        return toDoubleValue(str);
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
 * \brief replace all occurences of the toSearch string with toReplace modifying aString
 * \param str in which the replacement will occur
 * \param toSearch the search string
 * \param toReplace the replacement string
 */
void replaceInPlace(std::string& str, std::string_view toSearch, std::string_view toReplace);

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
void lowercaseInPlace(std::string& str);

/*!
 * \brief transform string to uppercase (in place version)
 * \param str input string that will be uppercased
 */
void uppercaseInPlace(std::string& str);

/*!
 * \brief check if a string starts with the provided string
 * \param str input string
 * \param search string
 * \return true is the input string starts with the search string
 */
bool startsWith(std::string_view str, std::string_view search);

/*!
 * \brief check if a string ends with the provided string
 * \param str input string
 * \param search string
 * \return true is the input string ends with the search string
 */
bool endsWith(std::string_view str, std::string_view search);

/*!
 * \brief create a trimmed view to a astring
 * \param str input string
 * \return trimmed view of the input string
 * \note it is important that the lifetime of the returned view is shorter than that of the input string
 */
[[nodiscard]] std::string_view trimmedView(std::string_view str);

/*!
 * \brief create a trimmed copy of a astring
 * \param str input string
 * \return trimmed copy of the input string
 */
[[nodiscard]] std::string trim(std::string_view str);
void trimInPlace(std::string& str);

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
            if (next(iter) != items.cend()) {
                ss << joinString;
            }
        }

        result = ss.str();
    } else {
        static_assert(dependent_false_v<ValueType>, "invalid invocation");
    }

    return result;
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
std::string join(const Container& items, const std::string& joinString, ToStringCb&& cb)
{
    std::ostringstream ss;
    for (auto iter = begin(items); iter != end(items); ++iter) {
        ss << cb(*iter);
        if (iter + 1 != end(items)) {
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

std::vector<std::string_view> splitView(std::string_view str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());
std::vector<std::string_view> splitView(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

template <typename ResultType, typename ConversionFunc>
std::vector<ResultType> splitTo(std::string_view str, char delimiter, ConversionFunc convFunc, Flags<SplitOpt> opt = Flags<SplitOpt>())
{
    auto splitted = splitView(str, delimiter, opt);

    std::vector<ResultType> result;
    result.reserve(splitted.size());
    for (auto& v : splitted) {
        result.emplace_back(convFunc(v));
    }

    return result;
}

template <typename ResultType, typename ConversionFunc>
std::vector<ResultType> splitTo(std::string_view str, std::string_view delimiter, ConversionFunc convFunc, Flags<SplitOpt> opt = Flags<SplitOpt>())
{
    auto splitted = splitView(str, delimiter, opt);

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
    : _result(str::splitView(src, sep, opt))
    {
    }

    Splitter(std::string_view src, char sep, Flags<SplitOpt> opt = Flags<SplitOpt>()) noexcept
    : _result(str::splitView(src, sep, opt))
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

} // namespace infra::str
