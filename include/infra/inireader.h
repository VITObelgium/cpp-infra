#pragma once

#include "infra/cast.h"
#include "infra/filesystem.h"
#include "infra/typetraits.h"

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
    IniReader(const fs::path& filename);
    IniReader(const IniReader&)            = default;
    IniReader(IniReader&&)                 = default;
    IniReader& operator=(const IniReader&) = default;
    IniReader& operator=(IniReader&&)      = default;

    // Return the list of sections found in ini file
    std::vector<std::string> sections() const;
    // Return all the key, values for the given section name
    std::unordered_map<std::string, std::string> section(std::string_view name) const;

    template <typename T>
    std::optional<T> get(std::string_view section, std::string_view name) const noexcept
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return optional_cast<std::string>(get_string(section, name));
        } else if constexpr (std::is_same_v<T, std::string_view>) {
            return get_string(section, name);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return get_uint32(section, name);
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return get_int32(section, name);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return get_uint64(section, name);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return get_int64(section, name);
        } else if constexpr (std::is_same_v<T, double>) {
            return get_double(section, name);
        } else if constexpr (std::is_same_v<T, float>) {
            return optional_cast<float>(get_double(section, name));
        } else if constexpr (std::is_same_v<T, bool>) {
            return get_bool(section, name);
        } else {
            static_assert(dependent_false<T>::value, "Unsupported ini type provided");
        }
    }

    std::optional<std::string_view> get_string(std::string_view section, std::string_view name) const noexcept;
    std::optional<int32_t> get_int32(std::string_view section, std::string_view name) const noexcept;
    std::optional<uint32_t> get_uint32(std::string_view section, std::string_view name) const noexcept;
    std::optional<int64_t> get_int64(std::string_view section, std::string_view name) const noexcept;
    std::optional<uint64_t> get_uint64(std::string_view section, std::string_view name) const noexcept;
    std::optional<double> get_double(std::string_view section, std::string_view name) const noexcept;
    std::optional<bool> get_bool(std::string_view section, std::string_view name) const noexcept;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _values;

    static int valueHandler(void* user, const char* section, const char* name, const char* value);
};
}
