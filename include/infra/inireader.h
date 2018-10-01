#pragma once

#include "infra/cast.h"

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace inf {

class IniReader
{
public:
    IniReader(const std::string& filename);
    IniReader(const IniReader&) = default;
    IniReader(IniReader&&)      = default;
    IniReader& operator=(const IniReader&) = default;
    IniReader& operator=(IniReader&&) = default;

    // Return the list of sections found in ini file
    std::vector<std::string> sections() const;

    template <typename T>
    std::optional<T> get(std::string_view section, std::string_view name) const noexcept
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return optional_cast<std::string>(getString(section, name));
        } else if constexpr (std::is_same_v<T, std::string_view>) {
            return getString(section, name);
        } else if constexpr (std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long>) {
            return getUnsignedInteger(section, name);
        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, long>) {
            return getInteger(section, name);
        } else if constexpr (std::is_same_v<T, double>) {
            return getReal(section, name);
        } else if constexpr (std::is_same_v<T, float>) {
            return optional_cast<float>(getReal(section, name));
        } else if constexpr (std::is_same_v<T, bool>) {
            return getBool(section, name);
        } else {
            static_assert(dependent_false<T>::value, "Unsupported ini type provided");
        }
    }

    std::optional<std::string_view>
    getString(std::string_view section, std::string_view name) const noexcept;
    std::optional<long> getInteger(std::string_view section, std::string_view name) const noexcept;
    std::optional<unsigned long> getUnsignedInteger(std::string_view section, std::string_view name) const noexcept;
    std::optional<double> getReal(std::string_view section, std::string_view name) const noexcept;
    std::optional<bool> getBool(std::string_view section, std::string_view name) const noexcept;

private:
    template <class TInternal>
    struct dependent_false : std::false_type
    {
    };

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _values;

    static int valueHandler(void* user, const char* section, const char* name, const char* value);
};
}
