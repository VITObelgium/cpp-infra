#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "infra/enumflags.h"
#include "internal/traits.h"

namespace infra::str {

bool containsValidInteger(std::string_view str);
bool containsValidFloatingPoint(std::string_view str);

void replace(std::string& aString, std::string_view toSearch, std::string_view toReplace);

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
            if (iter + 1 != items.cend()) {
                ss << joinString;
            }
        }

        result = ss.str();
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
std::vector<std::string> split(std::string_view str, const std::string& delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

std::vector<std::string_view> splitView(std::string_view str, char delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());
std::vector<std::string_view> splitView(std::string_view str, std::string_view delimiter, Flags<SplitOpt> opt = Flags<SplitOpt>());

inline constexpr Flags<SplitOpt> operator|(SplitOpt lhs, SplitOpt rhs) noexcept
{
    return Flags<SplitOpt>() | lhs | rhs;
}
}
