#pragma once

#include "infra/exception.h"

#include <algorithm>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vector>

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

template <typename Container, typename Predicate>
std::optional<typename Container::value_type> find_in_container_optional(const Container& c, Predicate&& pred) noexcept
{
    return as_optional(find_in_container(c, pred));
}

/* Search for an entry in the container that matches the predicate
 * The entry is returned as a reference
 * A RangeError exception is throw when there is no match
 */
template <typename Container, typename Predicate>
auto& find_in_container_required(Container&& c, Predicate&& pred)
{
    auto iter = std::find_if(c.begin(), c.end(), pred);
    if (iter == c.end()) {
        throw RangeError("No match found in the container");
    }

    return *iter;
}

template <typename Container>
auto& find_in_container_required(Container&& c, const typename std::decay_t<Container>::value_type& value)
{
    auto iter = std::find_if(c.begin(), c.end(), value);
    if (iter == c.end()) {
        throw RangeError("No match found in the container");
    }

    return *iter;
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

template <typename Container>
void remove_value_from_container(Container& c, const typename Container::value_type& value) noexcept
{
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
}

template <typename Container, typename Predicate>
void remove_from_container(Container& c, Predicate&& pred) noexcept
{
    c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

template <typename OutputContainer, typename T = typename OutputContainer::value_type>
void append_to_container(OutputContainer& output, std::initializer_list<T> values) noexcept
{
    output.reserve(output.size() + values.size());
    std::copy(values.begin(), values.end(), std::back_inserter(output));
}

template <typename OutputContainer, typename InputContainer>
void append_to_container(OutputContainer& output, const InputContainer& input) noexcept
{
    output.reserve(output.size() + input.size());
    std::copy(input.begin(), input.end(), std::back_inserter(output));
}

template <typename OutputContainer, typename InputContainer>
void insert_in_container(OutputContainer& output, const InputContainer& input) noexcept
{
    std::copy(input.begin(), input.end(), std::inserter(output, output.end()));
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

template <typename MapType>
auto find_in_map_required(MapType&& m, const typename std::decay_t<MapType>::key_type& key)
{
    auto iter = m.find(key);
    if (iter == m.end()) {
        throw RuntimeError("Key not found in map");
    }

    return iter->second;
}

/* Search for an entry in the map that matches the predicate
 * The entry is returned as a pointer (nullptr when not found)
 * This funtion never throws
 */
template <typename MapType>
auto find_in_map_optional(MapType&& m, const typename std::decay_t<MapType>::key_type& key) noexcept -> std::optional<typename std::decay_t<MapType>::mapped_type>
{
    if (auto iter = m.find(key); iter != m.end()) {
        return iter->second;
    }

    return {};
}

template <typename MapType>
std::vector<typename MapType::key_type> map_keys_as_vector(const MapType& m)
{
    std::vector<typename MapType::key_type> result;
    result.reserve(m.size());

    for (auto iter : m) {
        result.push_back(iter.first);
    }

    return result;
}

template <typename MapType>
std::vector<typename MapType::mapped_type> map_values_as_vector(const MapType& m)
{
    std::vector<typename MapType::mapped_type> result;
    result.reserve(m.size());

    for (auto iter : m) {
        result.push_back(iter.second);
    }

    return result;
}

template <typename TContainer, typename TVal = std::remove_const_t<typename TContainer::value_type>>
std::vector<TVal> container_as_vector(const TContainer& cont)
{
    return std::vector<TVal>(cont.begin(), cont.end());
}

template <typename TContainer, typename TVal = typename TContainer::value_type>
std::set<TVal> container_as_set(const TContainer& cont)
{
    return std::set<TVal>(cont.begin(), cont.end());
}

template <typename TContainer, typename TVal = typename TContainer::value_type>
std::unordered_set<TVal> container_as_unordered_set(const TContainer& cont)
{
    return std::set<TVal>(cont.begin(), cont.end());
}

/* Removes the constness of the iterator
 * /param c a non-const reference to the container
 * /param it a const iterator in the container
 */
template <typename Container, typename ConstIterator>
typename Container::iterator remove_constness(Container& c, ConstIterator it)
{
    return c.erase(it, it);
}

}
