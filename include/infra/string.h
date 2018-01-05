#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include "internal/traits.h"

namespace infra::str {

void replace(std::string& aString, std::string_view toSearch, std::string_view toReplace);

std::string lowercase(std::string_view str);
std::string uppercase(std::string_view str);

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
    std::string result;

    if (items.empty()) {
        return result;
    }

    if constexpr (can_cast_to_string_view_v<typename Container::value_type>) {
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
    } else if constexpr (is_streamable_v<typename Container::value_type>) {
        // Join implementation for objects that have a streaming operator implemented
        std::ostringstream ss;
        for (auto iter = items.cbegin(); iter != items.cend(); ++iter) {
            ss << *iter;
            if (iter + 1 != items.cend()) {
                ss << joinString;
            }
        }

        result = ss.str();
    } else {
        static_assert(false, "Items to join in container should be streamable or convertible to string_view")
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

inline std::vector<std::string> tokenize(std::string_view str, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    size_t pos   = 0;
    size_t index = 0;

    while ((pos = str.find(delimiter, index)) != std::string::npos) {
        tokens.emplace_back(str.substr(index, pos - index));
        index = pos + delimiter.size();
    }

    if (index < str.size()) {
        tokens.emplace_back(str.substr(index));
    }

    return tokens;
}
}
