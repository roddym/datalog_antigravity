#include "logger.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string name = "Antigravity";
    int runs = 3;

    std::cout << "--- Loop 1 ---" << "\n";
    for (int i = 0; i < runs; ++i) {
        // Expected behavior:
        // Iteration 0:
        //   "index" "name" "value"
        //   0 "Antigravity" 100
        // Iteration 1+:
        //   1 "Antigravity" 101
        //   ...
        LOG_KV("index", i, "name", name, "value", 100 + i);
    }

    std::cout << "\n--- Loop 2 (Different Call Site) ---" << "\n";
    for (int i = 0; i < 2; ++i) {
        // Should print header again because it's a different line
        LOG_KV("status", "running", "code", 200 + i);
    }

    return 0;
}
