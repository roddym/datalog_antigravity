#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <array>
#include <string_view>
#include <atomic>

namespace antigravity {
namespace detail {

// Helper to prepare arguments for printf
template <typename T>
auto prepare_arg(const T& arg) {
    if constexpr (std::is_same_v<T, std::string>) {
        return arg.c_str();
    } else {
        return arg;
    }
}

// Helper to deduce format specifier as string_view
template <typename T>
consteval std::string_view get_format_specifier() {
    if constexpr (std::is_arithmetic_v<T>) {
        if constexpr (std::is_integral_v<T>) {
            return "%d";
        } else {
            return "%f";
        }
    } else {
        return "%s";
    }
}

// Helper to deduce format specifier for keys (always strings)
consteval std::string_view get_key_format_specifier() {
    return "%s";
}

// Generator for compile-time format string
// IsKey=true: iterate 0, 2, 4... (Keys)
// IsKey=false: iterate 1, 3, 5... (Values)
template <bool IsKey, typename... Args>
consteval auto calculate_fmt_length() {
    constexpr std::size_t n_args = sizeof...(Args);
    constexpr std::size_t count = n_args / 2;
    std::size_t len = 0;
    
    auto calc_len = [&]<size_t I>(std::integral_constant<size_t, I>) {
        if (I > 0) len += 1; // " "
        
        constexpr size_t idx = IsKey ? (2 * I) : (2 * I + 1);
        using Type = std::tuple_element_t<idx, std::tuple<Args...>>;
        
        if constexpr (IsKey) {
             len += get_key_format_specifier().size();
        } else {
             len += get_format_specifier<Type>().size();
        }
    };

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (calc_len(std::integral_constant<std::size_t, Is>{}), ...);
    }(std::make_index_sequence<count>{});

    len += 1; // "\n"
    return len;
}

template <bool IsKey, typename... Args>
consteval auto generate_fmt() {
    constexpr std::size_t LEN = calculate_fmt_length<IsKey, Args...>();
    constexpr std::size_t n_args = sizeof...(Args);
    constexpr std::size_t count = n_args / 2;
    
    std::array<char, LEN + 1> arr{}; // +1 for null terminator
    auto it = arr.begin();
    
    auto append = [&](std::string_view sv) {
        for (char c : sv) *it++ = c;
    };
    
    auto fill = [&]<size_t I>(std::integral_constant<size_t, I>) {
        if (I > 0) append(" ");
        
        constexpr size_t idx = IsKey ? (2 * I) : (2 * I + 1);
        using Type = std::tuple_element_t<idx, std::tuple<Args...>>;
        
        if constexpr (IsKey) {
             append(get_key_format_specifier());
        } else {
             append(get_format_specifier<Type>());
        }
    };

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (fill(std::integral_constant<std::size_t, Is>{}), ...);
    }(std::make_index_sequence<count>{});
    
    append("\n");
    *it = '\0';
    
    return arr;
}

// Helper to print keys (indices 0, 2, 4...)
template <typename... Args>
void print_keys(const Args&... args) {
    static constexpr auto fmt_array = generate_fmt<true, Args...>();
    
    // Extract keys (0, 2, 4...) and prepare them for printf
    auto t = std::forward_as_tuple(args...);
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        std::printf(fmt_array.data(), prepare_arg(std::get<2*Is>(t))...);
    }(std::make_index_sequence<sizeof...(Args)/2>{});
}

// Helper to print values (indices 1, 3, 5...)
template <typename... Args>
void print_values(const Args&... args) {
    static constexpr auto fmt_array = generate_fmt<false, Args...>();

    // Extract values (1, 3, 5...) and prepare them for printf
    auto t = std::forward_as_tuple(args...);
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        std::printf(fmt_array.data(), prepare_arg(std::get<2*Is + 1>(t))...);
    }(std::make_index_sequence<sizeof...(Args)/2>{});
}

// Internal implementation logic called by the lambda
template <typename... Args>
void log_impl(bool& is_first, Args&&... args) {
    static_assert(sizeof...(args) % 2 == 0, "LOG_KV requires an even number of arguments (key-value pairs).");
    
    if (is_first) {
        print_keys(args...);
        is_first = false;
    }
    print_values(args...);
}

} // namespace detail
} // namespace antigravity

// Macro expands to an Immediately Invoked Lambda Expression (IILE).
// This guarantees that 'static bool is_first' is unique for each textual usage of LOG_KV.
#define LOG_KV(...) \
    [&](auto&&... args) { \
        static std::atomic<bool> is_first{true}; \
        if (is_first.exchange(false)) { \
            antigravity::detail::print_keys(args...); \
        } \
        antigravity::detail::print_values(args...); \
    }(__VA_ARGS__)

#endif // LOGGER_HPP
