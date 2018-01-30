#pragma once

#include "infra/exception.h"

#include <algorithm>
#include <optional>
#include <stdexcept>

namespace infra {

template <typename T>
std::optional<T> asOptional(const T* ptr)
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
const typename Container::value_type* findInContainer(const Container& c, Predicate&& pred) noexcept
{
    auto iter = std::find_if(c.begin(), c.end(), pred);
    if (iter == c.end()) {
        return nullptr;
    }

    return &(*iter);
}

template <typename Container, typename Predicate>
typename Container::value_type* findInContainer(Container& c, Predicate&& pred) noexcept
{
    auto iter = std::find_if(c.begin(), c.end(), pred);
    if (iter == c.end()) {
        return nullptr;
    }

    return &(*iter);
}

template <typename Container>
bool containerContains(const Container& c, const typename Container::value_type& value) noexcept
{
    auto iter = std::find(c.begin(), c.end(), value);
    return iter != c.end();
}

template <typename Container, typename Predicate>
void removeFromContainer(Container& c, Predicate&& pred) noexcept
{
    c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

template <typename Container, typename Predicate>
const typename Container::value_type* findInContainerOptional(const Container& c, Predicate&& pred) noexcept
{
    return asOptional(findInContainer(c, pred));
}

/* Search for an entry in the container that matches the predicate
 * The entry is returned as a reference
 * A RangeError exception is throw when there is no match
 */
template <typename Container, typename Predicate>
const typename Container::value_type& findInContainerRequired(const Container& c, Predicate&& pred)
{
    auto iter = std::find_if(c.begin(), c.end(), pred);
    if (iter == c.end()) {
        throw RangeError("No match found in the container");
    }

    return *iter;
}

/* Search for an entry in the map that matches the predicate
 * The entry is returned as a pointer (nullptr when not founc)
 * This funtion never throws
 */
template <typename MapType>
const typename MapType::mapped_type* findInMap(const MapType& m, const typename MapType::key_type& key) noexcept
{
    auto iter = m.find(key);
    if (iter == m.end()) {
        return nullptr;
    }

    return &(iter->second);
}

template <typename MapType>
typename MapType::mapped_type* findInMap(MapType& m, const typename MapType::key_type& key) noexcept
{
    auto iter = m.find(key);
    if (iter == m.end()) {
        return nullptr;
    }

    return &(iter->second);
}
}
