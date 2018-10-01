#pragma once

#include "infra/exception.h"

#include <algorithm>
#include <optional>
#include <stdexcept>

namespace inf {

template <typename T>
std::optional<T> as_optional(const T* ptr)
{
    if (ptr) {
        return std::make_optional<T>(*ptr);
    }

    return std::optional<T>();
}

/* Search for an entry in the container that matches the predicate
 * The entry is returned as a pointer (a nullptr is returned when there is no match)
 * This funtion never throws
 */
template <typename Container, typename Predicate>
auto find_in_container(Container&& c, Predicate&& pred) noexcept
{
    auto iter = std::find_if(c.begin(), c.end(), pred);
    if constexpr (std::is_pointer_v<typename std::decay_t<Container>::value_type>) {
        return iter == c.end() ? nullptr : *iter;
    } else {
        return iter == c.end() ? nullptr : &(*iter);
    }
}

template <typename Container>
bool container_contains(const Container& c, const typename Container::value_type& value) noexcept
{
    return std::find(begin(c), end(c), value) != end(c);
}

template <typename Container, typename Predicate>
bool container_contains_match(Container&& c, Predicate&& pred) noexcept
{
    return std::find_if(c.begin(), c.end(), pred) != c.end();
}

template <typename Container, typename Predicate>
void remove_from_container(Container& c, Predicate&& pred) noexcept
{
    c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

template <typename Container, typename Predicate>
const typename Container::value_type* find_in_container_optional(const Container& c, Predicate&& pred) noexcept
{
    return asOptional(findInContainer(c, pred));
}

/* Search for an entry in the container that matches the predicate
 * The entry is returned as a reference
 * A RangeError exception is throw when there is no match
 */
template <typename Container, typename Predicate>
const typename Container::value_type& find_in_container_required(const Container& c, Predicate&& pred)
{
    auto iter = std::find_if(c.begin(), c.end(), pred);
    if (iter == c.end()) {
        throw RangeError("No match found in the container");
    }

    return *iter;
}

/* Search for an entry in the map that matches the predicate
 * The entry is returned as a pointer (nullptr when not found)
 * This funtion never throws
 */
template <typename MapType>
auto find_in_map(MapType&& m, const typename std::decay_t<MapType>::key_type& key) noexcept
{
    auto iter = m.find(key);

    if constexpr (std::is_pointer_v<typename std::decay_t<MapType>::mapped_type>) {
        return iter == m.end() ? nullptr : iter->second;
    } else {
        return iter == m.end() ? nullptr : &(iter->second);
    }
}
}