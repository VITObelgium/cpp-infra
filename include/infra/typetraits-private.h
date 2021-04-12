#pragma once

#include <iosfwd>
#include <string_view>
#include <type_traits>

namespace inf {

// trait indicating that type T is castable to an std::string_view
template <typename T>
struct can_cast_to_string_view
{
private:
    template <typename U>
    static auto test(int) -> decltype(static_cast<std::string_view>(std::declval<U>()), std::true_type());

    template <typename>
    static std::false_type test(...);

public:
    static constexpr bool value = std::is_same_v<decltype(test<T>(0)), std::true_type>;
};

template <typename T>
inline constexpr bool can_cast_to_string_view_v = can_cast_to_string_view<T>::value;

// trait indicating that type T can be streamed to an std::ostream
template <typename T>
class is_streamable
{
    template <typename U>
    static auto test(int) -> decltype(std::declval<std::ostream&>() << std::declval<U>(), std::true_type());

    template <typename>
    static std::false_type test(...);

public:
    static const bool value = decltype(test<T>(0))::value;
};

template <typename T>
inline constexpr bool is_streamable_v = is_streamable<T>::value;

}
