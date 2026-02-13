#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <tuple>
#include <utility>

namespace antigravity {
namespace detail {

// Helper to print keys (indices 0, 2, 4...)
template <typename... Args>
void print_keys(const Args&... args) {
    auto t = std::forward_as_tuple(args...);
    constexpr std::size_t n = sizeof...(Args);
    
    bool first_field = true;
    auto process_idx = [&]<std::size_t I>(std::integral_constant<std::size_t, I>) {
        if (!first_field) std::cout << " ";
        std::cout << std::get<I>(t);
        first_field = false;
    };

    // Iterate 0, 2, 4... using a generated index sequence
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (process_idx(std::integral_constant<std::size_t, 2*Is>{}), ...);
    }(std::make_index_sequence<n/2>{});

    std::cout << "\n";
}

// Helper to print values (indices 1, 3, 5...)
template <typename... Args>
void print_values(const Args&... args) {
    auto t = std::forward_as_tuple(args...);
    constexpr std::size_t n = sizeof...(Args);

    bool first_field = true;
    auto process_idx = [&]<std::size_t I>(std::integral_constant<std::size_t, I>) {
        if (!first_field) std::cout << " ";
        std::cout << std::get<I>(t);
        first_field = false;
    };

    // Iterate 1, 3, 5...
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (process_idx(std::integral_constant<std::size_t, 2*Is + 1>{}), ...);
    }(std::make_index_sequence<n/2>{});

    std::cout << "\n";
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
        static bool is_first = true; \
        antigravity::detail::log_impl(is_first, args...); \
    }(__VA_ARGS__)

#endif // LOGGER_HPP
