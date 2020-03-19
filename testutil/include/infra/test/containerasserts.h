#pragma once

#include "infra/string.h"

#include <cmath>
#include <string>
#include <doctest/doctest.h>
#include <fmt/format.h>
#include <unordered_map>

namespace inf::test {

template <typename Container>
static std::string print_container(const Container& c)
{
    return inf::str::join(c, ", ", [](const auto v) {
        return fmt::format("{}", v);
    });
}

template <typename Map>
static std::string print_map(const Map& m)
{
    std::stringstream ss;
    for (auto& [key, value] : m) {
        ss << fmt::format("[{}]: {}\n", key, value);
    }

    return ss.str();
}

template <typename Container1, typename Container2>
static std::string print_container_data(const Container1& lhs, const Container2& rhs)
{
    std::stringstream ss;
    ss << "--- Expected data:\n"
       << print_container(lhs)
       << "\n--- Actual data:\n"
       << print_container(rhs);

    return ss.str();
}

template <typename Map>
static std::string print_map_data(const Map& lhs, const Map& rhs)
{
    std::stringstream ss;
    ss << "--- Expected data:\n"
       << print_map(lhs)
       << "\n--- Actual data:\n"
       << print_map(rhs);

    return ss.str();
}

template <typename TKey, typename TValue>
static std::string print_container_diff(const std::unordered_map<TKey, TValue>& lhs, const std::unordered_map<TKey, TValue>& rhs, double)
{
    if (lhs.size() <= 50) {
        return print_map_data(lhs, rhs);
    }

    return "TODO";
}

template <typename Container1, typename Container2>
static std::string print_container_diff(const Container1& lhs, const Container2& rhs, double /*tolerance*/)
{
    if (lhs.size() <= 50) {
        return print_container_data(lhs, rhs);
    }

    return "TODO";

    // std::for_each(begin(lhs), end(lhs), begin(rhs), [](auto v1, auto v2) {
    //     using WidestType = decltype(v1 + v2);

    //     if constexpr (std::numeric_limits<WidestType>::has_quiet_NaN) {
    //         equal = gdx::cpu::float_equal_to<WidestType>(static_cast<WidestType>(tolerance))(static_cast<WidestType>(v1), static_cast<WidestType>(v2));
    //     } else {
    //         (void)tolerance;
    //         equal = static_cast<WidestType>(v1) == static_cast<WidestType>(v2);
    //     }
    // });
}

template <typename Container1, typename Container2>
bool container_eq(const Container1& c1, const Container2& c2)
{
    REQUIRE_MESSAGE(size(c1) == size(c2), fmt::format("Container sizes do not match: {} vs {}", size(c1), size(c2)));

    return std::equal(begin(c1), end(c1), begin(c2), [](auto a, auto b) -> bool {
        if constexpr (std::is_floating_point_v<decltype(a)>) {
            if (std::isnan(a)) { return std::isnan(b); }
            if (std::isnan(b)) { return false; }
        }

        return a == b;
    });
}

template <typename TKey, typename TValue>
bool container_eq(const std::unordered_map<TKey, TValue>& m1, const std::unordered_map<TKey, TValue>& m2)
{
    REQUIRE_MESSAGE(size(m1) == size(m2), fmt::format("Container sizes do not match: {} vs {}", size(m1), size(m2)));

    for (auto& [key, value] : m1) {
        if (auto iter = m2.find(key); iter != m2.end()) {
            if constexpr (std::is_floating_point_v<TValue>) {
                if (std::isnan(value)) {
                    if (!std::isnan(iter->second)) {
                        return false;
                    }
                } else if (std::isnan(iter->second)) {
                    return false;
                }

                if (iter->second != value) {
                    return false;
                }
            }
        } else {
            return false;
        }
    }

    return true;
}

}

#define CHECK_CONTAINER_EQ(lhs, rhs) \
    CHECK_MESSAGE(inf::test::container_eq((lhs), (rhs)), inf::test::print_container_diff((lhs), (rhs), 0.0));

