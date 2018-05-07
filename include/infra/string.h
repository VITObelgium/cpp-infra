#pragma once

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "infra/enumflags.h"
#include "infra/internal/traits.h"
#include "infra/typetraits.h"

namespace infra::str {

bool containsValidInteger(std::string_view str);
bool containsValidFloatingPoint(std::string_view str);

std::optional<int32_t> toInt32(std::string_view str) noexcept;
std::optional<int64_t> toInt64(std::string_view str) noexcept;
std::optional<float> toFloat(std::string_view str) noexcept;
std::optional<double> toDouble(std::string_view str) noexcept;

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

bool iequals(std::string_view str1, std::string_view str2);
void replaceInPlace(std::string& aString, std::string_view toSearch, std::string_view toReplace);
[[nodiscard]] std::string replace(std::string_view aString, std::string_view toSearch, std::string_view toReplace);

[[nodiscard]] std::string lowercase(std::string_view str);
[[nodiscard]] std::string uppercase(std::string_view str);

void lowercaseInPlace(std::string& str);
void uppercaseInPlace(std::string& str);

bool startsWith(std::string_view aString, std::string_view search);
bool endsWith(std::string_view aString, std::string_view search);

[[nodiscard]] std::string_view trimmedView(std::string_view str);
[[nodiscard]] std::string trim(std::string_view str);
void trimInPlace(std::string& str);

// Join items in the container with the provided join string
// e.g.: join(std::vector<std::string>({"one", "two"}), ", ") == "one, two"
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
        // This implementation is roughly 10 times faster the ostream version for strings

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

// Join implementation for objects where the tostring implementation is provided as a callable
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

enum class SplitOpt
{
    NoEmpty = 1 << 0,
    Trim    = 1 << 1
};

std::vector<std::string> split(std::string_view str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());
std::vector<std::string> split(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

std::vector<std::string_view> splitView(std::string_view str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());
std::vector<std::string_view> splitView(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

template <typename ResultType, typename ConversionFunc>
std::vector<ResultType> splitTo(std::string_view str, char delimiter, ConversionFunc convFunc)
{
    auto splitted = splitView(str, delimiter);

    std::vector<ResultType> result;
    result.reserve(splitted.size());
    for (auto& v : splitted) {
        result.emplace_back(convFunc(v));
    }

    return result;
}

template <typename ResultType, typename ConversionFunc>
std::vector<ResultType> splitTo(std::string_view str, std::string_view delimiter, ConversionFunc convFunc)
{
    auto splitted = splitView(str, delimiter);

    std::vector<ResultType> result;
    result.reserve(splitted.size());
    for (auto& v : splitted) {
        result.emplace_back(convFunc(v));
    }

    return result;
}

inline constexpr Flags<SplitOpt> operator|(SplitOpt lhs, SplitOpt rhs) noexcept
{
    return Flags<SplitOpt>() | lhs | rhs;
}

class Splitter
{
public:
    static const char* WhiteSpaceSeparator;

    Splitter(std::string_view src, std::string sep);

    class const_iterator
    {
    public:
        using value_type        = std::string_view;
        using pointer           = const value_type*;
        using reference         = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        explicit const_iterator(const Splitter& sp);
        explicit const_iterator();

        const_iterator& operator++();
        const_iterator operator++(int);

        reference operator*();
        pointer operator->();

        bool operator!=(const const_iterator& other) const noexcept;

    private:
        const Splitter* _splitter;
        std::string_view _value;
    };

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;

    std::string_view next() const noexcept;
    bool finished() const noexcept;

private:
    std::string_view _src;
    std::string _sep;
    mutable size_t _pos    = 0;
    mutable bool _finished = false;
};

} // namespace infra::str
